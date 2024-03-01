/*
 * Project: Software Defined Blocks
 * Copyright (C) 2024 alf.labs gmail com.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define HTTPD_SOCK_ERR_FAIL      -1
#define HTTPD_SOCK_ERR_INVALID   -2
#define HTTPD_SOCK_ERR_TIMEOUT   -3

typedef void* httpd_handle_t;

enum http_method {
    HTTP_GET,
    HTTP_POST,
};

typedef enum http_method httpd_method_t;

typedef struct httpd_req {
    size_t content_len;
    void *user_ctx;
} httpd_req_t;

typedef struct httpd_config {
    unsigned    task_priority;
    BaseType_t  core_id;
//
//    /**
//     * TCP Port number for receiving and transmitting HTTP traffic
//     */
//    uint16_t    server_port;
//
//    /**
//     * UDP Port number for asynchronously exchanging control signals
//     * between various components of the server
//     */
//    uint16_t    ctrl_port;
//
//    uint16_t    max_open_sockets;   /*!< Max number of sockets/clients connected at any time*/
//    uint16_t    max_uri_handlers;   /*!< Maximum allowed uri handlers */
//    uint16_t    max_resp_headers;   /*!< Maximum allowed additional headers in HTTP response */
//    uint16_t    backlog_conn;       /*!< Number of backlog connections */
//    bool        lru_purge_enable;   /*!< Purge "Least Recently Used" connection */
//    uint16_t    recv_wait_timeout;  /*!< Timeout for recv function (in seconds)*/
//    uint16_t    send_wait_timeout;  /*!< Timeout for send function (in seconds)*/
//
//    /**
//     * Global user context.
//     *
//     * This field can be used to store arbitrary user data within the server context.
//     * The value can be retrieved using the server handle, available e.g. in the httpd_req_t struct.
//     *
//     * When shutting down, the server frees up the user context by
//     * calling free() on the global_user_ctx field. If you wish to use a custom
//     * function for freeing the global user context, please specify that here.
//     */
//    void * global_user_ctx;
//
//    /**
//     * Free function for global user context
//     */
//    httpd_free_ctx_fn_t global_user_ctx_free_fn;
//
//    /**
//     * Global transport context.
//     *
//     * Similar to global_user_ctx, but used for session encoding or encryption (e.g. to hold the SSL context).
//     * It will be freed using free(), unless global_transport_ctx_free_fn is specified.
//     */
//    void * global_transport_ctx;
//
//    /**
//     * Free function for global transport context
//     */
//    httpd_free_ctx_fn_t global_transport_ctx_free_fn;
//
//    bool enable_so_linger;  /*!< bool to enable/disable linger */
//    int linger_timeout;     /*!< linger timeout (in seconds) */
//
//    /**
//     * Custom session opening callback.
//     *
//     * Called on a new session socket just after accept(), but before reading any data.
//     *
//     * This is an opportunity to set up e.g. SSL encryption using global_transport_ctx
//     * and the send/recv/pending session overrides.
//     *
//     * If a context needs to be maintained between these functions, store it in the session using
//     * httpd_sess_set_transport_ctx() and retrieve it later with httpd_sess_get_transport_ctx()
//     *
//     * Returning a value other than ESP_OK will immediately close the new socket.
//     */
//    httpd_open_func_t open_fn;
//
//    /**
//     * Custom session closing callback.
//     *
//     * Called when a session is deleted, before freeing user and transport contexts and before
//     * closing the socket. This is a place for custom de-init code common to all sockets.
//     *
//     * Set the user or transport context to NULL if it was freed here, so the server does not
//     * try to free it again.
//     *
//     * This function is run for all terminated sessions, including sessions where the socket
//     * was closed by the network stack - that is, the file descriptor may not be valid anymore.
//     */
//    httpd_close_func_t close_fn;
//
//    /**
//     * URI matcher function.
//     *
//     * Called when searching for a matching URI:
//     *     1) whose request handler is to be executed right
//     *        after an HTTP request is successfully parsed
//     *     2) in order to prevent duplication while registering
//     *        a new URI handler using `httpd_register_uri_handler()`
//     *
//     * Available options are:
//     *     1) NULL : Internally do basic matching using `strncmp()`
//     *     2) `httpd_uri_match_wildcard()` : URI wildcard matcher
//     *
//     * Users can implement their own matching functions (See description
//     * of the `httpd_uri_match_func_t` function prototype)
//     */
//    httpd_uri_match_func_t uri_match_fn;
} httpd_config_t;

#define HTTPD_DEFAULT_CONFIG() { tskIDLE_PRIORITY, PRO_CPU }

typedef struct httpd_uri {
    const char    *uri;
    httpd_method_t method;
    esp_err_t (*handler)(httpd_req_t *r);
    void *user_ctx;
} httpd_uri_t;

int httpd_req_recv(httpd_req_t *r, char *buf, size_t buf_len) {
    return HTTPD_SOCK_ERR_FAIL;
}

esp_err_t  httpd_start(httpd_handle_t *handle, const httpd_config_t *config) {
    return ESP_ERR_INVALID_ARG;
}

esp_err_t httpd_stop(httpd_handle_t handle) {
    return ESP_ERR_INVALID_ARG;
}

esp_err_t httpd_register_uri_handler(httpd_handle_t handle,
                                     const httpd_uri_t *uri_handler) {
    return ESP_ERR_INVALID_ARG;
}

esp_err_t httpd_resp_set_type(httpd_req_t *r, const char *type) {
    return ESP_ERR_INVALID_ARG;
}

esp_err_t httpd_resp_set_hdr(httpd_req_t *r, const char *field, const char *value) {
    return ESP_ERR_INVALID_ARG;
}

esp_err_t httpd_resp_send(httpd_req_t *r, const char *buf, ssize_t buf_len) {
    return ESP_ERR_INVALID_ARG;
}

esp_err_t httpd_resp_sendstr(httpd_req_t *r, const char *str) {
    return ESP_ERR_INVALID_ARG;
}

esp_err_t httpd_resp_send_408(httpd_req_t *r) {
    return ESP_ERR_INVALID_ARG;
}

esp_err_t httpd_resp_send_500(httpd_req_t *r) {
    return ESP_ERR_INVALID_ARG;
}

size_t httpd_req_get_url_query_len(httpd_req_t *r) {
    return 0;
}

esp_err_t httpd_req_get_url_query_str(httpd_req_t *r, char *buf, size_t buf_len) {
    return ESP_ERR_NOT_FOUND;
}

esp_err_t httpd_query_key_value(const char *qry, const char *key, char *val, size_t val_size) {
    return ESP_ERR_NOT_FOUND;
}
