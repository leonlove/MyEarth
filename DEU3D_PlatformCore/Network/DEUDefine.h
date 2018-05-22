#ifndef _DEUNETWORK_DEUDEFINE_H_
#define _DEUNETWORK_DEUDEFINE_H_

enum DEU_ERROR_TYPE
{
    DEU_SUCCESS               = -8000000,//  成功
    DEU_PART_SUCCESS          = 0,      //  部分成功
    DEU_INVALID_NETWORK       = 1,      //  网络有问题
    DEU_INVALID_REQUEST_PARAM = 2,      //  参数不正确
    DEU_INVALID_URL_FORMAT    = 3,      //  URL格式不正确
    DEU_FAIL_CONNECTION       = 4,      //  连接到服务器失败
    DEU_FAIL_SEND_REQUEST     = 5,      //  发送请求到服务器失败
    DEU_FAIL_RESPONSE_LENGTH  = 6,      //  得到response长度失败
    DEU_FAIL_RESPONSE_CONTENT = 7,      //  得到response内容失败
    DEU_ERROR_RESPONSE_STATE  = 8,      //  错误的返回状态
    

    DEU_FAIL_GET_RCD          = 101,    //  获取散列信息失败
    DEU_RECEIVE_NO_PARAM      = 102,    //  从服务端获取到的信息没有需要的参数
    DEU_EMPTY_PORTS           = 103,    //  端口为空
    DEU_INVALID_ID            = 104,    //  无效的ID
    DEU_FAIL_DATA_PORT        = 105,    //  启动/停止服务失败

    DEU_EMPTY_DATA            = 151,    //  输入数据为空

    DEU_INVALID_BSON_FROMAT   = 200,    //  bson格式无效
    DEU_LAYER_CONTAINSELF     = 201,    //  图层自包含
    DEU_FAIL_READ_BLOCK       = 202,    //  从deudb中读取失败
    DEU_FAIL_UPDATE_BLOCK     = 203,    //  更新数据失败
    DEU_FAIL_REMOVE_BLOCK     = 204,    //  从deudb中删除失败
    DEU_FAIL_GET_TYPE         = 205,    //  获取类型失败
    DEU_FAIL_GET_DB_AND_ID    = 206,    //  获取db和id失败
    DEU_FAIL_CREATE_DEUDB     = 207,    //  创建deudb失败
    DEU_FAIL_OPEN_DEUDB       = 208,    //  打开deudb失败
    DEU_FAIL_GET_POST_DATA    = 209,    //  获取post数据失败
    DEU_EMPTY_POST_DATA       = 210,    //  空的post数据
    DEU_INVALID_POST_DATA     = 211,    //  无效的post数据
    DEU_FAIL_CONN_MYSQL       = 212,    //  无法连接mysql
    DEU_FAIL_SELECT_DB        = 213,    //  选择database失败
    DEU_FAIL_CREATE_DB        = 214,    //  创建database
    DEU_FAIL_CREATE_TABLE     = 215,    //    创建表失败
    DEU_FAIL_DELETE_DEUDB     = 216,    //  删除数据集失败
    DEU_FAIL_DELETE_ATTR      = 217,    //  删除数据集属性失败
    DEU_FAIL_ADD_ATTR         = 218,    //  添加属性失败
    DEU_FAIL_UPDATE_ATTR      = 219,    //  更新属性失败
    DEU_READ_EMPTY_DATA       = 220,    //  读取的数据为空
    DEU_FAIL_QUERY_ID         = 221,    //  根据属性查ID失败
    DEU_FAIL_OPEN_FILE        = 222,    //  打开文件失败
    DEU_FAIL_GET_PORT         = 223,    //  获取端口失败
    DEU_FAIL_READ_CONF        = 224,    //  读取配置文件失败
    DEU_EMPTY_QUERY_STRING    = 225,    //  空的请求串
    DEU_FAIL_ADD_BLOCK        = 226,    //  添加数据块失败 
    DEU_FAIL_ADD_VIRTUAL_TILE = 227,    //  添加虚拟瓦片失败
    DEU_ALREADY_EXIST         = 228,    //  数据已经存在
    DEU_FAIL_REPLACE_BLOCK    = 229,    //  替换数据失败
    DEU_FAIL_GET_VERSION      = 230,    //  获取版本失败
    DEU_FAIL_ADD_CHILD        = 231,    //  添加儿子ID失败
    DEU_PARENT_NOT_EXIST      = 232,    //  父图层不存在
    DEU_CHILD_NOT_EXIST       = 233,    //  子图层不存
    DEU_NOT_CATEGORY_ID       = 234,    //  不是符号库ID
    DEU_NOT_LAYER_ID          = 235,    //  不是图层ID
    DEU_NO_ENOUGH_DISK_SPACE  = 236,    //  分配空间失败，请检查是否磁盘空间不足

    DEU_FAIL_CALCU_EXTENT     = 301,    //  计算瓦片范围失败
    DEU_FAIL_INIT_INTERFACE   = 302,    //  初始化接口失败
    DEU_FAIL_GET_INTERFACE    = 303,    //  获取接口失败
    DEU_FAIL_QUERY_OUT_DATA   = 304,    //  获取外部数据失败
    DEU_FAIL_SERVER_INFO      = 305,    //  根据数据集代码获取服务信息失败
    DEU_FAIL_INIT_SERVER      = 306,    //  初始化服务失败

    DEU_FAIL_USER_PWD         = 401,    //  获取用户名和密码失败
    DEU_ERROR_PASSWD          = 402,    //  用户名或密码错误
    DEU_FAIL_MAKE_TICKET      = 403,    //  生成票据失败
    DEU_FAIL_GET_ENTRY        = 404,    //  获取用户条目错误
    DEU_INVALID_TICKET        = 405,    //  无效票据,请重新登录
    DEU_NO_PERMISSION         = 406,    //  没有相应的权限
    DEU_FAIL_GET_TICKET       = 407,    //  获取票据失败
    DEU_ALREADY_LOGIN         = 408,    //  已有用户登录，切换用户请先将当前用户注销
    DEU_USER_NOT_LOGIN        = 409,    //  该用户没有登录

    DEU_UNKNOWN               = 400     //  未知
};

struct DEUData
{
    ID       m_strID;
    void*    m_pData;
    unsigned m_nLength;

    DEUData()
    {
        m_pData = NULL;
        m_nLength = 0;
    }
};

#endif  //_DEUNETWORK_DEUDEFINE_H_