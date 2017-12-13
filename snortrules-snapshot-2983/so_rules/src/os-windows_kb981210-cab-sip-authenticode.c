/*
 * This code was automatically generated by the Sourcefire VRT Rules2C web generator
 *
 * We make no claims to the correctness or fitness of the code generated or of modifications
 * to the generated code.
 *
 * Use at your own risk.
 *
 * Please report any problems with this tool to research@sourcefire.com
 */

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

#include "so-util.h"

/* declare detection functions */
int rule16530eval(void *p);

/* declare rule data structures */
/* flow:established, to_client; */
static FlowFlags rule16530flow0 = 
{
    FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule16530option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &rule16530flow0
    }
};
#ifndef CONTENT_FAST_PATTERN
#define CONTENT_FAST_PATTERN 0
#endif
// content:"MSCF|00 00 00 00|", payload raw, depth 0, fast_pattern; 
static ContentInfo rule16530content1 = 
{
    (uint8_t *) "MSCF|00 00 00 00|", /* pattern (now in snort content format) */
    8, /* depth */
    0, /* offset */
    CONTENT_FAST_PATTERN|CONTENT_BUF_NORMALIZED, /* flags */
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule16530option1 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule16530content1
    }
};

// file_data;
static CursorInfo rule16530file_data2 =
{
    0, /* offset */
    CONTENT_BUF_NORMALIZED /* flags */
};

static RuleOption rule16530option2 =
{
#ifndef MISSINGFILEDATA
    OPTION_TYPE_FILE_DATA,
#else
    OPTION_TYPE_SET_CURSOR,
#endif
    {
        &rule16530file_data2
    }
};

/* references for sid 16530 */
/* reference: cve "2010-0487"; */
static RuleReference rule16530ref1 = 
{
    "cve", /* type */
    "2010-0487" /* value */
};

/* reference: url "technet.microsoft.com/en-us/security/bulletin/MS10-019"; */
static RuleReference rule16530ref2 = 
{
    "url", /* type */
    "technet.microsoft.com/en-us/security/bulletin/MS10-019" /* value */
};

static RuleReference *rule16530refs[] =
{
    &rule16530ref1,
    &rule16530ref2,
    NULL
};
/* metadata for sid 16530 */
/* metadata:service http, policy balanced-ips drop, policy security-ips drop; */
static RuleMetaData rule16530service1 = 
{
    "service http"
};


//static RuleMetaData rule16530policy1 = 
//{
//    "policy balanced-ips drop"
//};
//
//static RuleMetaData rule16530policy2 = 
//{
//    "policy security-ips drop"
//};


static RuleMetaData *rule16530metadata[] =
{
    &rule16530service1,
//    &rule16530policy1,
//    &rule16530policy2,
    NULL
};

RuleOption *rule16530options[] =
{
    &rule16530option0,
    &rule16530option1,
    &rule16530option2,
    NULL
};

Rule rule16530 = {
   /* rule header, akin to => tcp any any -> any any */
   {
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "$HTTP_PORTS", /* SRCPORT   */
   
       0, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
   
       "any", /* DSTPORT   */
   },
   /* metadata */
   { 
       3,  /* genid */
       16530, /* sigid */
       9, /* revision */
       "attempted-user", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "OS-WINDOWS CAB SIP authenticode alteration attempt",     /* message */
       rule16530refs /* ptr to references */
       ,rule16530metadata
   },
   rule16530options, /* ptr to rule options */
   &rule16530eval, /* replace with NULL to use the built in detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule16530eval(void *p) {
    const uint8_t *cursor_normal = 0;
    SFSnortPacket *sp = (SFSnortPacket *) p;

    const uint8_t *beg_of_payload, *end_of_payload;
    uint32_t cbCabinet;
    uint32_t coffCabStart;
    uint16_t cFolders;
    uint16_t flags;
    uint16_t cbCFHeader = 0;
    uint8_t cbCFFolder = 0;
    uint32_t skip = 0;

    int i;

    if(sp == NULL)
        return RULE_NOMATCH;

    if(sp->payload == NULL)
        return RULE_NOMATCH;
    
    // flow:established, to_client;
    if (checkFlow(p, rule16530options[0]->option_u.flowFlags) > 0 ) {

        // file_data;
        #ifndef MISSINGFILEDATA
        if (fileData(p, rule16530options[2]->option_u.cursor, &cursor_normal) <= 0) {
        #else
        if (setCursor(p, rule16530options[2]->option_u.cursor, &cursor_normal) <= 0) {
        #endif
            return RULE_NOMATCH;
        }


        // content:"MSCF|00 00 00 00|", payload raw, depth 0, fast_pattern;
        if (contentMatch(p, rule16530options[1]->option_u.content, &cursor_normal) > 0) {
            if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_payload, &end_of_payload) <= 0)
               return RULE_NOMATCH;

            //Make sure we have enough bytes to read the header plus 4 bytes
            if (cursor_normal + 31 > end_of_payload)
                return RULE_NOMATCH;

            //Extract file size
            cbCabinet = read_little_32(cursor_normal);

            //Extract number of folder structures
            cFolders = read_little_16(cursor_normal+18);

            //If there are no folder structures, we really don't care, so bail
            if (cFolders == 0)
                return RULE_NOMATCH;

            //Extract the flags field
            //This field tells us what optional data is present in the header
            flags = read_little_16(cursor_normal+22);

            //Jump over the required part of the header
            cursor_normal += 28;

            //If flag is set, there is optional reserved header data
            if (flags & 0x4) {

                if(cursor_normal + 3 > end_of_payload)
                    return RULE_NOMATCH;

                //There are 4 bytes of additional fields plus a variable abReserve field
                //cbCFHeader tells us how big abReserved is
                //we need it to know how many bytes to skip
                cbCFHeader = read_little_16(cursor_normal);

                //cbCFFolder tells us how big the optional header data in each FOLDER structure is
                //This option data is only there if the above flag is set, so we extract it here
                cbCFFolder = *(cursor_normal+2);

                //The size of this optional part of the header is 4 bytes plus abReserve
                skip += cbCFHeader + 4;

                //Skip ahead
                cursor_normal += skip;
            }

            //First vulnerable condition is if the combined size of all the FOLDER structures
            //exceeds the total file size. Check this.
            //Sum of sizes of all FOLDER structures is (8 + optional data)* number of folders
            if ((uint32_t)((cbCFFolder + 8) * cFolders) > cbCabinet)
                return RULE_MATCH;

            //More optional data. If this flag is set, there are two string fields present each of max size 255 plus one null byte
            //We don't care about these, skip over them.
            if (flags & 0x1) {
                for (i = 0; (cursor_normal < end_of_payload) && (*cursor_normal++ != 0); i++) {
                    if (i > 256)
                        return RULE_NOMATCH;
                  }
                for (i = 0; (cursor_normal < end_of_payload) && (*cursor_normal++ != 0); i++) {
                    if (i > 256)
                        return RULE_NOMATCH;
                }

            }

            //Might be two more strings. Skip skip skip!
            if (flags & 0x2) {
                for (i = 0; (cursor_normal < end_of_payload) && (*cursor_normal++ != 0); i++) {
                    if (i > 256)
                        return RULE_NOMATCH;
                }
                for (i = 0; (cursor_normal < end_of_payload) && (*cursor_normal++ != 0); i++) {
                    if (i > 256)
                        return RULE_NOMATCH;
                } 
            }

            //We're now at the first FOLDER structure. Set the amount to skip.
            //This should be the size of the FOLDER structure.
            skip = 8 + cbCFFolder;

            //In a loop, as the number of Folders decrements:
            //   read the coffCabStart field
            //   compare it to the file size
            //   if it's bigger, alert
            //   otherwise check that we can read 4 bytes of the next FOLDER
            //   skip to the beginning of the next FOLDER and do it all again.
            while (cFolders-- > 0) {
                //Make sure we can read at least 4 bytes
                if (cursor_normal + 4 > end_of_payload)
                    return RULE_NOMATCH;

                coffCabStart = read_little_32(cursor_normal);

                if (coffCabStart > cbCabinet)
                    return RULE_MATCH;

                cursor_normal += skip;
            }

        }
    }
    return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule16530,
    NULL
};
*/


