/** @file   rauschen.h
 *  @date   Oct 6, 2015
 *  @Author Hannes Rauhe (hannes.rauhe@sap.com)
 */

#ifndef SRC_API_RAUSCHEN_H_
#define SRC_API_RAUSCHEN_H_

typedef enum {
  RAUSCHEN_STATUS_TIMEOUT = -1,
  RAUSCHEN_STATUS_OK = 0
} rauschen_status;

typedef struct {
  const char* text;
  const char* sender;
  const char* type;
} rauschen_message_t;

typedef struct {
  int num;
} rauschen_handle_t;

#ifdef __cplusplus
extern "C"
{
#endif

rauschen_status rauschen_add_host(const char* hostname);

rauschen_status rauschen_send_message(const char* message, const char* message_type, const char* receiver);

rauschen_handle_t* rauschen_register_message_handler(const char* message_type);

rauschen_message_t* rauschen_get_next_message(const rauschen_handle_t* handle);

rauschen_status rauschen_free_message(rauschen_message_t* message);

rauschen_status rauschen_unregister_message_handler(rauschen_handle_t* handle);

#ifdef __cplusplus
}
#endif

#endif /* SRC_API_RAUSCHEN_H_ */
