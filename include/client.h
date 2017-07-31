#pragma once

// Standard Libraries
#include <time.h>
#include <stdint.h>

// Other Libraries
#include <pthread.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

// Core Headers
#include "discord.h"
#include "websocket.h"

// forward declaration
typedef struct discord_client discord_client_t;

/**************************************************
	Users
		user_info_t

		get_user_name
		get_user_id
		get_user_discrim
		get_user_info
**************************************************/

typedef struct {
	uint64_t _user_id;
	uint16_t _discriminator;
	const char* _name;
	uint32_t _name_len;
	discord_client_t* _client;
} user_info_t;

LIB_EXPORT const char* get_user_name(user_info_t* user, int* length);
LIB_EXPORT uint64_t get_user_id(user_info_t* user);
LIB_EXPORT uint16_t get_user_discrim(user_info_t* user);

LIB_EXPORT user_info_t *get_user_info(discord_client_t* client, uint64_t id);

/**************************************************
	Messages
		message_t
**************************************************/

typedef struct {
	uint64_t _channel_id;
	uint64_t _message_id;
	const char* _contents;
	user_info_t* _sender;
} message_t;


/**************************************************
	Callbacks
		client_callback_*
		discord_client_callbacks_t
**************************************************/

#define CALLBACK_TYPE(NAME, RET, ...) typedef RET (*client_callback_ ## NAME)(discord_client_t* client, __VA_ARGS__)
#define CALLBACK_TYPE_VOID(NAME, RET) typedef RET (*client_callback_ ## NAME)(discord_client_t* client)

CALLBACK_TYPE_VOID(connected, void);
CALLBACK_TYPE(unhandled_dispatch, void, char* dispatch_str, cJSON* root);
CALLBACK_TYPE(connection_error, int, uint16_t code, const char* reason);
CALLBACK_TYPE(latency_updated, int, double oldLatency, double newLatency);
CALLBACK_TYPE(message_receive, int, message_t* message);


typedef struct {
	client_callback_unhandled_dispatch	on_unhandled_dispatch;
	client_callback_connected			on_connected;
	client_callback_connection_error	on_connection_error;
	client_callback_latency_updated		on_latency_update;
	client_callback_message_receive		on_message_receive;
} discord_client_callbacks_t;

/**************************************************
	Client (client.c)
		struct discord_client

		discord_create_client
		discord_free_client

		client_connect
		client_disconnect
		client_send_message
**************************************************/

// this is typedef'd as discord_client_t up top
struct discord_client {
	char* _token;

	double _latency;
	int _heartbeat_interval;
	int _seq;
	struct timespec* _heartbeat_start;
	uint8_t _send_heartbeats;

	CURLM* _curl_handle;
	pthread_t* _heartbeat_thread;
	pthread_t* _gateway_thread;

	client_websocket_t* _client_socket;
	discord_client_callbacks_t* _callbacks;
};

LIB_EXPORT discord_client_t* discord_create_client(discord_client_callbacks_t* callbacks, const char* token);
LIB_EXPORT void discord_free_client(discord_client_t* client);

LIB_EXPORT void client_connect(discord_client_t* client);
LIB_EXPORT void client_disconnect(discord_client_t* client);

LIB_EXPORT void client_send_message(discord_client_t* client, uint64_t channel_id, const char* contents);
