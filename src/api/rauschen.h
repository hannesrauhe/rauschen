/** @file   rauschen.h
 *  @date   Oct 6, 2015
 *  @Author Hannes Rauhe (hannes.rauhe@sap.com)
 */

#ifndef SRC_API_RAUSCHEN_H_
#define SRC_API_RAUSCHEN_H_

typedef enum {
  RAUSCHEN_STATUS_OK = 0
} rauschen_status;

#ifdef __cplusplus
extern "C"
{
#endif

rauschen_status rauschen_add_host(const char* hostname);

rauschen_status rauschen_send_message(const char* message, const char* message_type, const char* receiver);

#ifdef __cplusplus
}
#endif

#endif /* SRC_API_RAUSCHEN_H_ */