#ifndef SIPHELPERS_H
#define SIPHELPERS_H
#include <osipparser2/osip_message.h>
#include <string>
#include <string.h>
using std::string;
using std::to_string;
static void setCharValue(char** src, const char* tar) {
    if(*src != NULL) {
        osip_free(*src);
    }
    *src = (char*)osip_malloc(strlen(tar) + 2);
    strcpy(*src, tar);
}

static void deleteCharValue(char **src) {
    if(*src != NULL)
        osip_free(*src);
    *src = NULL;
}


static string ALLOWS[] = {"INVITE", "REGISTER", "SUBCRIBE", "ACK", "CANCEL", "PRACK", "UPDATE", "BYE", "MESSAGE", "OPTIONS", "NOTIFY", "REFER", "INFO", "PING"};
static osip_list_t sipGenerateAllows() {
    osip_list_t result;
    osip_list_init(&result);
    for(int i = 0;i < 14;i ++) {
        osip_allow_t* all;
        osip_allow_init(&all);
        setCharValue(&all->value, ALLOWS[i].c_str());
        osip_list_add(&result, all, -1);
    }
    return result;
}

static void sipSetVia(osip_message_t* sip, string& ip, int listenPort) {
    osip_via_t *via;
    osip_message_get_via(sip, osip_list_size(&sip->vias)-1, &via);
    setCharValue(&via->host, ip.c_str());
    setCharValue(&via->port, to_string(listenPort).c_str());
    osip_generic_param_add(&via->via_params, osip_strdup("rport"), NULL);
}

static void sipSetDomains(osip_message_t* sip, const char* tar) {
    if(sip->req_uri != NULL) {
        setCharValue(&sip->req_uri->host, tar);
        deleteCharValue(&sip->req_uri->port);
    }
	setCharValue(&sip->from->url->host, tar);
	setCharValue(&sip->to->url->host, tar);
	deleteCharValue(&sip->from->url->port);
	deleteCharValue(&sip->to->url->port);
}

static void sipRemoveIMSI(osip_message_t* sip) {
    if(strlen(sip->from->url->username) > 15)
        strcpy(sip->from->url->username, sip->from->url->username + 4);
    if(strlen(sip->to->url->username) > 15)
		strcpy(sip->to->url->username, sip->to->url->username + 4);

}

static void sipAddHeaders(osip_message_t* sip) {
   	while(osip_list_size(&sip->headers) > 1) {
    	osip_list_remove(&sip->headers, 0);
	}
	osip_header_t* head;
	osip_header_init(&head);
	setCharValue(&head->hname, "K");
	setCharValue(&head->hvalue, "100rel");
	osip_list_add(&sip->headers, head, -1);
	osip_header_init(&head);
	setCharValue(&head->hname, "K");
	setCharValue(&head->hvalue, "replaces");
	osip_list_add(&sip->headers, head, -1);
	osip_header_init(&head);
	setCharValue(&head->hname, "Max-Forwards");
	setCharValue(&head->hvalue, "70");
	osip_list_add(&sip->headers, head, -1);
}

static void sipSetContact(osip_message_t* sip, string& ip, int port) {
    osip_contact_t* contact;
	osip_header_t* head;
   	if(osip_list_size(&sip->contacts) > 0) {
		cout<<"Changing contacts"<<endl;
		contact = (osip_contact_t*)osip_list_get(&sip->contacts, 0);
		while(osip_list_size(&contact->gen_params) > 0) {
    		cout<<"Try to remove tail"<<endl;
			osip_list_remove(&contact->gen_params, 0);
		}
		cout<<"Remove tails done"<<endl;
		if(contact->url != NULL) {
			setCharValue(&contact->url->host, ip.c_str());
			setCharValue(&contact->url->port, to_string(port).c_str());
			if(contact->url->username[0] == 'I')
				strcpy(contact->url->username, contact->url->username + 4);
		}
		setCharValue(&contact->displayname, (string("\"sip:") + string(sip->from->url->username) + string("@") + string(sip->from->url->host) + "\"").c_str());				
		osip_header_init(&head);
		setCharValue(&head->hname, "Expires");
		setCharValue(&head->hvalue, "3600");
		osip_list_add(&sip->headers, head, -1);
		cout<<"change contacts done"<<endl;
	    setCharValue(&contact->url->port, to_string(port).c_str());
	}
}

static void sipRemoveRoute(osip_message_t* sip) {
    if(MSG_IS_REGISTER(sip)) {
        while(osip_list_size(&sip->routes) > 0) {
            osip_list_remove(&sip->routes, 0);
        }
    }
}

static bool sipHasSDP(osip_message_t* sip) {
    return sip->content_type != NULL && strcmp("sdp", sip->content_type->subtype) == 0;
}

static string sipGetSDP(osip_message_t* sip) {
    if(!sipHasSDP(sip))
        return "";
    osip_body_t* body = (osip_body_t*) osip_list_get(&sip->bodies, 0);
    return string(body->body);

}

static void sipSetSDP(osip_message_t* sip, string& sdp) {
    osip_body_t* body = (osip_body_t*) osip_list_get(&sip->bodies, 0);
	osip_free(body->body);
	body->body = (char*)osip_malloc(sdp.length() + 1);
	strcpy(body->body, sdp.c_str());
	body->length = sdp.length();
}


#endif
