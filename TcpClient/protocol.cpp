#include "protocol.h"
#include <stdlib.h>
#include <string.h>

PDU* mkPDU(uint uiMsgLen) {
    uint len = sizeof(PDU) + uiMsgLen;
    PDU* pdu = reinterpret_cast<PDU*>(malloc(len));
    if(pdu == nullptr) {
        exit(EXIT_FAILURE);
    }
    memset(pdu, 0, len);
    pdu->uiPDULen = len;
    pdu->uiMsgLen = uiMsgLen;
    return pdu;
}
