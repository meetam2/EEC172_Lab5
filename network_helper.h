

#ifndef __NETWORKH_H__
#define __NETWORKH_H__

//NEED TO UPDATE THIS FOR IT TO WORK!
#define DATE                2    /* Current Date */
#define MONTH               6     /* Month 1-12 */
#define YEAR                2025  /* Current year */
#define HOUR                13    /* Time - hours */
#define MINUTE              317    /* Time - minutes */
#define SECOND              0     /* Time - seconds */


#define APPLICATION_NAME      "SSL"
#define APPLICATION_VERSION   "SQ24"
#define SERVER_NAME           "a1caql46tkmrrp-ats.iot.us-east-2.amazonaws.com" // CHANGE ME
#define GOOGLE_DST_PORT       8443


#define POSTHEADER "POST /things/meetam_cc3200/shadow HTTP/1.1\r\n"             // CHANGE ME
#define HOSTHEADER "Host: a1caql46tkmrrp-ats.iot.us-east-2.amazonaws.com\r\n"  // CHANGE ME
#define CHEADER "Connection: Keep-Alive\r\n"
#define CTHEADER "Content-Type: application/json; charset=utf-8\r\n"
#define CLHEADER1 "Content-Length: "
#define CLHEADER2 "\r\n\r\n"

#define DATA1 "{" \
            "\"state\": {\r\n"                                              \
                "\"desired\" : {\r\n"                                       \
                    "\"default\" :\""                                           \
                        "Hello phone, "                                     \
                        "message from CC3200 via AWS IoT!"                  \
                        "\"\r\n"                                            \
                "}"                                                         \
            "}"                                                             \
        "}\r\n\r\n"


#define DATAHEAD "{" \
            "\"state\": {\r\n"                                              \
                "\"desired\" : {\r\n"                                       \

// Variables here: e.g. " "default" : ... , "var": ... "

#define DATAFOOT "\r\n"                                            \
                "}"                                                         \
            "}"                                                             \
        "}\r\n\r\n"


#define GETHEADER "GET /things/meetam_cc3200/shadow HTTP/1.1\r\n"

int initialize_network();
int set_time();
int http_post(int iTLSSockID);
int http_postmsg(int iTLSSockID, char *message);
int http_get(int iTLSSockID);

#endif //  __GPIOIF_H__
