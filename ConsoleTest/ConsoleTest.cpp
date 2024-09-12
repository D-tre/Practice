#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/types.h>
#include <getopt.h>
#include <linux/netfilter.h>            /* for NF_ACCEPT */
#include<time.h>
#include "UtilData.h"

//#define GEN_DATA


#include <libnetfilter_queue/libnetfilter_queue.h>

#define IPT_TCP (6)
#define IPT_UDP (17)

struct ip_hdr {
    uint8_t vhl;
    uint8_t tos;
    uint16_t len;
    uint16_t id;
    uint16_t off;
    uint8_t ttl;
    uint8_t proto;
    uint16_t sum;
    uint16_t src[2];
    uint16_t dst[2];
};

struct tcp_hdr {
    uint16_t sport;
    uint16_t dport;
    unsigned int seq;
    unsigned int ack;
    uint8_t off;
    uint8_t flags;
    uint16_t win;
    uint16_t sum;
    uint16_t urp;
};

struct udp_hdr {
    uint16_t sport;
    uint16_t dport;
    uint16_t length;
    uint16_t sum;
};

#define IP_HL(ip)   (((ip)->vhl) & 0x0f)
#define TH_OFF(th)  (((th)->off & 0xf0) >> 4)
#define TH_OFF_UDP (8)

struct rule_t {
    uint8_t* val1;
    uint8_t* val2;
    int length;
    struct rule_t* next;
};

struct rule_t* rules = NULL;
int verbose = 0;
int queue_num = 0;
int recycle = 0;
int recyl81 = 0;

void usage()
{
    fprintf(stderr, "Usage: nfqsed [-s /val1/val2] [-x /hex1/hex2] [-f file] [-v] [-q num]\n"
        "  -s val1/val2     - replaces occurences of val1 with val2 in the packet payload\n"
        "  -x hex1/hex2     - replaces occurences of hex1 with hex2 in the packet payload\n"
        "  -f file          - read replacement rules from the specified file\n"
        "  -q num           - bind to queue with number 'num' (default 0)\n"
        "  -v               - be verbose\n");
    exit(1);
}

void print_rule(const struct rule_t* rule)
{
    int i = 0;
    for (i = 0; i < rule->length; i++) {
        printf("%02x", rule->val1[i]);
    }
    printf(" -> ");
    for (i = 0; i < rule->length; i++) {
        printf("%02x", rule->val2[i]);
    }
    printf("\n");
}

void add_rule(const char* rule_str)
{
    char delim = rule_str[0];
    char* pos = NULL;
    int length = 0;
    struct rule_t* rule;
    if (strlen(rule_str) < 4) {
        fprintf(stderr, "rule too short: %s\n", rule_str);
        exit(1);
    }
    pos = strchr(rule_str + 1, delim);
    if (!pos) {
        fprintf(stderr, "incorrect rule: %s\n", rule_str);
        exit(1);
    }
    length = strlen(pos + 1);
    if (pos - rule_str - 1 != length) {
        fprintf(stderr, "val1 and val2 must be the same length: %s\n", rule_str);
        exit(1);
    }
    rule = malloc(sizeof(struct rule_t));
    rule->val1 = malloc(length);
    memcpy(rule->val1, rule_str + 1, length);
    rule->val2 = malloc(length);
    memcpy(rule->val2, pos + 1, length);
    rule->length = length;
    rule->next = NULL;
    if (rules) {
        rule->next = rules;
        rules = rule;
    }
    else {
        rules = rule;
    }
}

void str_to_hex(const char* str, uint8_t* dest, int length)
{
    for (int i = 0; i < length; i++) {
        int ret = sscanf(str, "%2hhx", &dest[i]);
        if (ret != 1) {
            fprintf(stderr, "Incorrect hex byte: %c%c\n", str[0], str[1]);
            exit(1);
        }
        str += 2;
    }
}

void add_hex_rule(const char* rule_str)
{
    char delim = rule_str[0];
    char* pos = NULL;
    int length = 0;
    struct rule_t* rule;
    if (strlen(rule_str) < 6) {
        fprintf(stderr, "hex rule too short: %s\n", rule_str);
        exit(1);
    }
    pos = strchr(rule_str + 1, delim);
    if (!pos) {
        fprintf(stderr, "incorrect hex rule: %s\n", rule_str);
        exit(1);
    }
    length = strlen(pos + 1);
    if (length % 2 == 1) {
        fprintf(stderr, "hex rule values must have even length\n");
        exit(1);
    }
    if (pos - rule_str - 1 != length) {
        fprintf(stderr, "hex1 and hex2 must have the same length: %s\n", rule_str);
        exit(1);
    }
    length /= 2;
    rule = malloc(sizeof(struct rule_t));
    rule->val1 = malloc(length);
    str_to_hex(rule_str + 1, rule->val1, length);
    rule->val2 = malloc(length);
    str_to_hex(pos + 1, rule->val2, length);
    rule->length = length;
    rule->next = NULL;
    if (rules) {
        rule->next = rules;
        rules = rule;
    }
    else {
        rules = rule;
    }
}

void load_rules(const char* rules_file)
{
    FILE* f;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    f = fopen(rules_file, "r");
    if (!f) {
        fprintf(stderr, "cannot open %s", rules_file);
        exit(1);
    }
    while ((read = getline(&line, &len, f)) != -1) {
        if (line[0] == '#' || read == 1) {
            // skip comments and empty lines
            continue;
        }
        if (line[read - 1] == '\n') {
            line[read - 1] = 0;
        }
        add_rule(line);
    }
    free(line);
    fclose(f);
}

uint16_t csum(uint16_t proto, uint16_t len, uint16_t* src_addr, uint16_t* dest_addr, uint8_t* buff)
{
    uint32_t sum = 0;
    int i = 0;

    sum += ntohs(src_addr[0]);
    sum += ntohs(src_addr[1]);
    sum += ntohs(dest_addr[0]);
    sum += ntohs(dest_addr[1]);
    sum += len;
    sum += proto;
    for (i = 0; i < (len / 2); i++) {
        sum += ntohs((buff[i * 2 + 1] << 8) | buff[i * 2]);
    }
    if ((len % 2) == 1) {
        sum += buff[len - 1] << 8;
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    sum = ~sum;
    return htons((uint16_t)sum);
}

uint8_t* find(const struct rule_t* rule, uint8_t* payload, int payload_length)
{
    int rule_len = rule->length;
    int i = 0, j = 0, match = 0;
    for (i = 0; i < payload_length - rule_len + 1; i++) {
        match = 1;
        for (j = 0; j < rule_len; j++) {
            if (payload[i + j] != rule->val1[j]) {
                match = 0;
                break;
            }
        }
        if (match) {
            return payload + i;
        }
    }
    return NULL;
}

static int cb(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg,
    struct nfq_data* nfa, void* data)
{
    int id = 0, len = 0;
    struct nfqnl_msg_packet_hdr* ph;
    uint8_t* payload = NULL, * proto_payload, * pos;
    struct ip_hdr* ip;
    struct tcp_hdr* tcp;
    struct udp_hdr* udp;
    uint16_t ip_size = 0, proto_size = 0;
    struct rule_t* rule = rules;

    ph = nfq_get_msg_packet_hdr(nfa);
    if (ph) {
        id = ntohl(ph->packet_id);
    }
    len = nfq_get_payload(nfa, &payload);
    if (len < 0) {
        fprintf(stderr, "Error getting payload\n");
        return len;
    }
    ip = (struct ip_hdr*)payload;
    ip_size = IP_HL(ip) * 4;

    if (ip->proto == IPT_TCP) {
        // Try to match TCP packets
        tcp = (struct tcp_hdr*)(payload + ip_size);
        proto_size = TH_OFF(tcp) * 4;
    }
    else if (ip->proto == IPT_UDP) {
        // Try to match UDP packets
        udp = (struct udp_hdr*)(payload + ip_size);
        proto_size = TH_OFF_UDP;
    }
    else {
        printf("Do not ACCEPT");
        // Do not accept protos other than TCP & UDP
        return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
    }
    int proto_len = len - ip_size - proto_size;
    proto_payload = (uint8_t*)(payload + ip_size + proto_size);
    if ((proto_len == 66 || proto_len == 67) && proto_payload[0] != 0x43)
    {
        printf("*\n*\n*\n762 data\n*\n*\n*\n");
#ifdef  GEN_DATA
        printf("\n%d", proto_len);
        for (int i = 0; i < proto_len; ++i)
        {
            if (i % 16 == 0)
                printf("\n");
            printf("0x%02x, ", proto_payload[i]);
        }
#else
        if (proto_len == 67)
        {
            for (int i = 0; i < proto_len; ++i)
            {
                proto_payload[i] = cha67[i];
            }
        }
#endif
    }
    if (proto_len > 70) {
        if (proto_payload[0] == 0x01)
            printf("****************************01 data start**********************************\n");
        if (proto_payload[0] == 0x01) {
            if (proto_payload[3] == 0x00) {
                if (proto_payload[4] == 0xDA || proto_payload[4] == 0xAE || proto_payload[4] == 0xD6 || proto_payload[4] == 0xA6) {
                    for (int i = 0; i < proto_len; i++) {
                        if (i % 16 == 0)
                            printf("\n");
                        if (((int)proto_payload[i] == 33 || (int)proto_payload[i] == 29 || proto_payload[i] == 0x0b) && i > 5 && (int)proto_payload[i - 1] == 0 && (int)proto_payload[i - 2] == 0) {
                            proto_payload[i + 12] = 0x49 | proto_payload[proto_len - 1];
                            proto_payload[i + 13] = 0x45 | proto_payload[proto_len - 1];
                            proto_payload[i + 14] = 0x50 | proto_payload[proto_len - 1];
                        }
                        if (i > 150 && i % 4 == 0) {
                            proto_payload[i] = 0x00;
                        }
                        printf("%02x   ", proto_payload[i]);
                    }
                }
                else {
                    printf("%02x   ", proto_payload[4]);
                    return nfq_set_verdict(qh, id, NF_DROP, len, payload);
                }
            }
            else if (proto_payload[3] == 0x01 || proto_payload[3] == 0x02 || proto_payload[3] == 0x03 || proto_payload[3] == 0x04) {
                for (int i = 0; i < proto_len; i++) {
                    if (i % 16 == 0)
                        printf("\n");
                    if (((int)proto_payload[i] == 33 || (int)proto_payload[i] == 29 || proto_payload[i] == 0x0b) && i > 5 && (int)proto_payload[i - 1] == 0 && (int)proto_payload[i - 2] == 0) {
                        proto_payload[i + 12] = 0x49 | proto_payload[proto_len - 1];
                        proto_payload[i + 13] = 0x45 | proto_payload[proto_len - 1];
                        proto_payload[i + 14] = 0x50 | proto_payload[proto_len - 1];
                    }
                    if (i > 120) {
                        proto_payload[i] = 0x00;
                    }
                    printf("%02x   ", proto_payload[i]);
                }
            }
            else {
                printf("new detect \n");
                /*for (int i = 0; i < proto_len; i++) {
                    if (i % 16 == 0)
                        printf("\n");
                    if (((int)proto_payload[i] == 33 || (int)proto_payload[i] == 29 || proto_payload[i] == 0x0b) && i > 5 && (int)proto_payload[i - 1] == 0 && (int)proto_payload[i - 2] == 0) {
                        proto_payload[i] = 0x00;
                        proto_payload[i + 1] = 0x00;
                        proto_payload[i + 2] = 0x00;
                        proto_payload[i + 3] = 0x00;
                        proto_payload[i + 4] = 0x00;
                        proto_payload[i + 5] = 0x00;
                    }
                    if (i > 150) {
                        if (i % 4 == 0)
                            proto_payload[i] = 0x00;
                    }
                    printf("%02x   ", proto_payload[i]);
                }
                for (int i = 0; i < proto_len; i++) {
                    if (i % 16 == 0)
                        printf("\n");
                    printf("%02x   ", proto_payload[i]);
                }*/
                return nfq_set_verdict(qh, id, NF_DROP, len, payload);
            }
        }
        else if (proto_payload[0] == 0x00) {
            printf("queye der lenf:  %d\n", proto_len);

            if (proto_len == 359) {
                printf("normally   \n");
                /*for (int i = 0; i < proto_len; i++) {
                    if (i % 16 == 0)
                        printf("\n");
                    printf("%02x   ", proto_payload[i]);
                    if (proto_payload[i] != 0x00)
                        proto_payload[i] = cha[i];
                }*/
            }
            else {
                printf("unnormally   \n*******************************\n");
                for (int i = 0; i < proto_len; i++) {
                    if (i % 16 == 0)
                        printf("\n");
                    proto_payload[i] = proto_payload[i] & 0x56;
                    printf("%02x   ", proto_payload[i]);
                }
            }
        }
        else if (proto_payload[0] == 0x00)
            return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
        else if (proto_payload[0] == 0x16)
            return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
        else {

#ifdef  GEN_DATA
            printf("\n%d", proto_len);
            for (int i = 0; i < proto_len; ++i)
            {
                if (i % 16 == 0)
                    printf("\n");
                printf("0x%02x, ", proto_payload[i]);
            }
#else
            switch (proto_len)
            {
            case 74: {
                printf("\n***************data   74*****************");
                for (int i = 0; i < proto_len; i++)
                {
                    proto_payload[i] = cha74[i];
                }
                break;
            }
            case 94: {
                printf("\n***************data   94*****************");
                for (int i = 0; i < proto_len; i++)
                {
                    proto_payload[i] = cha94[i];
                }
                break;
            }
            case 1024: {
                printf("\n***************data   1024*****************");
                for (int i = 0; i < proto_len; i++)
                {
                    proto_payload[i] = cha1024[i];
                }
                break;
            }
            case 1366: {
                printf("\n***************data   1366*****************");
                for (int i = 0; i < proto_len; i++)
                {
                    proto_payload[i] = cha1366[i];
                }
                break;
            }
            default: {
                printf("\n****data %d***********\n", proto_len);
                for (int i = 0; i < proto_len; i++)
                {
                    if (i % 16 == 0)
                        printf("\n");
                    printf("0x%02x, ", proto_payload[i]);
                }
                return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
                break;
            }
            }
#endif
        }
    }


    printf("\n");
    //flag=0;
    if (ip->proto == IPT_TCP) {
        // Reset TCP checksum
        tcp->sum = 0;
        tcp->sum = csum(IPT_TCP, len - ip_size, ip->src, ip->dst, (uint8_t*)tcp);
    }
    else {
        // Reset UDP checksum
        udp->sum = 0;
        udp->sum = csum(IPT_UDP, len - ip_size, ip->src, ip->dst, (uint8_t*)udp);
    }
    return nfq_set_verdict(qh, id, NF_ACCEPT, len, payload);
}

void read_queue()
{
    struct nfq_handle* h;
    struct nfq_q_handle* qh;
    int fd;
    int rv;
    char buf[4096] __attribute__((aligned));

    printf("opening library handle\n");
    h = nfq_open();
    if (!h) {
        fprintf(stderr, "error during nfq_open()\n");
        exit(1);
    }

    printf("unbinding existing nf_queue handler for AF_INET (if any)\n");
    if (nfq_unbind_pf(h, AF_INET) < 0) {
        fprintf(stderr, "error during nfq_unbind_pf()\n");
        exit(1);
    }

    printf("binding nfnetlink_queue as nf_queue handler for AF_INET\n");
    if (nfq_bind_pf(h, AF_INET) < 0) {
        fprintf(stderr, "error during nfq_bind_pf()\n");
        exit(1);
    }

    printf("binding this socket to queue '%d'\n", queue_num);
    qh = nfq_create_queue(h, queue_num, &cb, NULL);
    if (!qh) {
        fprintf(stderr, "error during nfq_create_queue()\n");
        exit(1);
    }

    printf("setting copy_packet mode\n");
    if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
        fprintf(stderr, "can't set packet_copy mode\n");
        exit(1);
    }

    fd = nfq_fd(h);
    while ((rv = recv(fd, buf, sizeof(buf), 0)) && rv >= 0) {
        if (verbose) {
            //printf("packet received\n");
        }
        nfq_handle_packet(h, buf, rv);
    }

    printf("unbinding from queue 0\n");
    nfq_destroy_queue(qh);

#ifdef INSANE
    /* normally, applications SHOULD NOT issue this command, since
     * it detaches other programs/sockets from AF_INET, too ! */
    printf("unbinding from AF_INET\n");
    nfq_unbind_pf(h, AF_INET);
#endif 

    printf("closing library handle\n");
    nfq_close(h);
}

int main(int argc, char* argv[])
{
#ifdef GEN_DATA
    printf("数据产生模式！！！！！！！！！\n");
#endif // GEN_DATA

    verbose = 1;
    queue_num = atoi("23");
    if (verbose) {
        struct rule_t* rule = rules;
        printf("Rules (in hex):\n");
        while (rule) {
            printf("  ");
            print_rule(rule);
            rule = rule->next;
        }
    }
    read_queue();
    return 0;
}
