#ifndef PROTOCOL_H
#define PROTOCOL_H

typedef unsigned int uint;

#define REGIST_OK "regist ok"
#define REGIST_FAIL "regist fail"

#define LOGIN_OK "login ok"
#define LOGIN_FAIL "login fail"

#define UNKNOWN_ERROR "unknown error"

#define CAN_NOT_ADD_FRIEND "can not add friend"
#define ALREADY_FRIEND "already friend"

#define DELETE_FRIEND_SUCCESS "delete friend success"

enum MSG_TYPE {
    ENUM_MSG_TYPE_MIN = 0,
    ENUM_MSG_TYPE_REGIST_REQUEST,   // 注册请求
    ENUM_MSG_TYPE_REGIST_RESPONSE,  // 注册响应

    ENUM_MSG_TYPE_LOGIN_REQUEST,    // 登录请求
    ENUM_MSG_TYPE_LOGIN_RESPONSE,   // 登录响应

    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,    // 获取在线用户请求
    ENUM_MSG_TYPE_ALL_ONLINE_RESPONSE,   // 获取在线用户响应

    ENUM_MSG_TYPE_SEARCH_USER_REQUEST,    // 查找用户请求
    ENUM_MSG_TYPE_SEARCH_USER_RESPONSE,   // 查找用户响应

    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST,    // 添加好友请求(Client->Server->Client)
    ENUM_MSG_TYPE_ADD_FRIEND_RESPONSE,   // 添加好友响应(Server->Client)

    ENUM_MSG_TYPE_AGREE_FRIEND_RESPONSE,    // 添加好友的同意请求(Client->Server->Client)
    ENUM_MSG_TYPE_REFUSE_FRIEND_RESPONSE,   // 添加好友的拒绝响应(Client->Server->Client)

    ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST,     // 刷新好友请求
    ENUM_MSG_TYPE_FLUSH_FRIEND_RESPONSE,    // 刷新好友响应

    ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST,     // 删除好友请求
    ENUM_MSG_TYPE_DELETE_FRIEND_RESPONSE,    // 删除好友响应

    ENUM_MSG_TYPE_CHAT_REQUEST,             // 私聊请求
    ENUM_MSG_TYPE_CHAT_OK_RESPONSE,         // 私聊响应, 发送成功
    ENUM_MSG_TYPE_CHAT_FAIL_RESPONSE,       // 私聊响应, 发送失败

    ENUM_MSG_TYPE_BROADCAST_FRIEND_MESSAGE, // 好友广播消息

    ENUM_MSG_TYPE_DIR_CREATE_REQUEST,   // 创建文件夹请求
    ENUM_MSG_TYPE_DIR_CREATE_RESPONSE,  // 创建文件夹响应

    ENUM_MSG_TYPE_DIR_FLUSH_REQUEST,   // 刷新文件夹请求
    ENUM_MSG_TYPE_DIR_FLUSH_RESPONSE,  // 刷新文件夹响应

    ENUM_MSG_TYPE_DIR_DELETE_REQUEST,   // 删除文件夹请求
    ENUM_MSG_TYPE_DIR_DELETE_RESPONSE,  // 删除文件夹响应

    ENUM_MSG_TYPE_DIR_RENAME_REQUEST,   // 重命名文件夹请求
    ENUM_MSG_TYPE_DIR_RENAME_RESPONSE,  // 重命名文件夹响应

    ENUM_MSG_TYPE_DIR_GO_REQUEST,   // 进入文件夹请求
    ENUM_MSG_TYPE_DIR_GO_RESPONSE,  // 进入文件夹响应

    ENUM_MSG_TYPE_DIR_GOBACK_REQUEST,   // 进入上一级文件夹请求
    ENUM_MSG_TYPE_DIR_GOBACK_RESPONSE,  // 进入上一级文件夹响应

    ENUM_MSG_TYPE_FILE_UPLOAD_REQUEST,   // 上传文件请求
    ENUM_MSG_TYPE_FILE_UPLOAD_RESPONSE,  // 上传文件响应

    ENUM_MSG_TYPE_FILE_UPLOAD_CONTINUE,  // 上传文件正在传输
    ENUM_MSG_TYPE_FILE_UPLOAD_END,       // 上传文件传输完毕

    ENUM_MSG_TYPE_FILE_DELETE_REQUEST,   // 删除文件请求
    ENUM_MSG_TYPE_FILE_DELETE_RESPONSE,  // 删除文件响应

    ENUM_MSG_TYPE_FILE_DOWNLOAD_REQUEST,   // 下载文件请求
    ENUM_MSG_TYPE_FILE_DOWNLOAD_RESPONSE,  // 下载文件响应

    ENUM_MSG_TYPE_FILE_DOWNLOAD_CONTINUE,   // 下载文件正在传输
    ENUM_MSG_TYPE_FILE_DOWNLOAD_END,        // 下载文件传输完毕

    ENUM_MSG_TYPE_FILE_SHARE_REQUEST,   // 分享文件请求
    ENUM_MSG_TYPE_FILE_SHARE_RESPONSE,  // 分享文件响应
    ENUM_MSG_TYPE_FILE_SHARE_END,       // 分享文件完成

    ENUM_MSG_TYPE_MAX = 0x00ffffff,
};

/// 协议数据单元
struct PDU {
    uint uiPDULen;      // 协议数据单元大小
    uint uiMsgType;     // 消息类型
    char caData[64];    // 文件名
    uint uiMsgLen;      // 消息长度
    int caMsg[];        // 消息数据域
};

enum FILE_TYPE {
    ENUM_FILE_TYPE_FILE = 1,    // 文件
    ENUM_FILE_TYPE_DIR  = 2,    // 文件夹
    ENUM_FILE_TYPE_EXEC = 4,    // 可执行
};

/// 文件信息单元
struct FileInfoUnit {
    char fileName[32];
    int fileType;
};

// 创建一个PDU
PDU* mkPDU(uint uiMsgLen);

#endif // PROTOCOL_H
