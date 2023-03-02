#include "ubusfuncs.h"

int connect_to_ubus(struct ubus_context **ctx)
{
    *ctx = ubus_connect(NULL);
    if(ctx == NULL){
        return -1;
    }
    return 0;
}
void disconnect_from_ubus(struct ubus_context *ctx)
{
    ubus_free(ctx);
}