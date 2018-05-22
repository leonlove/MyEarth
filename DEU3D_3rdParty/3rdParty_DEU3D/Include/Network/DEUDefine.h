#ifndef _DEUNETWORK_DEUDEFINE_H_
#define _DEUNETWORK_DEUDEFINE_H_

enum DEU_ERROR_TYPE
{
    DEU_SUCCESS               = -8000000,//  �ɹ�
    DEU_PART_SUCCESS          = 0,      //  ���ֳɹ�
    DEU_INVALID_NETWORK       = 1,      //  ����������
    DEU_INVALID_REQUEST_PARAM = 2,      //  ��������ȷ
    DEU_INVALID_URL_FORMAT    = 3,      //  URL��ʽ����ȷ
    DEU_FAIL_CONNECTION       = 4,      //  ���ӵ�������ʧ��
    DEU_FAIL_SEND_REQUEST     = 5,      //  �������󵽷�����ʧ��
    DEU_FAIL_RESPONSE_LENGTH  = 6,      //  �õ�response����ʧ��
    DEU_FAIL_RESPONSE_CONTENT = 7,      //  �õ�response����ʧ��
    DEU_ERROR_RESPONSE_STATE  = 8,      //  ����ķ���״̬
    

    DEU_FAIL_GET_RCD          = 101,    //  ��ȡɢ����Ϣʧ��
    DEU_RECEIVE_NO_PARAM      = 102,    //  �ӷ���˻�ȡ������Ϣû����Ҫ�Ĳ���
    DEU_EMPTY_PORTS           = 103,    //  �˿�Ϊ��
    DEU_INVALID_ID            = 104,    //  ��Ч��ID
    DEU_FAIL_DATA_PORT        = 105,    //  ����/ֹͣ����ʧ��

    DEU_EMPTY_DATA            = 151,    //  ��������Ϊ��

    DEU_INVALID_BSON_FROMAT   = 200,    //  bson��ʽ��Ч
    DEU_LAYER_CONTAINSELF     = 201,    //  ͼ���԰���
    DEU_FAIL_READ_BLOCK       = 202,    //  ��deudb�ж�ȡʧ��
    DEU_FAIL_UPDATE_BLOCK     = 203,    //  ��������ʧ��
    DEU_FAIL_REMOVE_BLOCK     = 204,    //  ��deudb��ɾ��ʧ��
    DEU_FAIL_GET_TYPE         = 205,    //  ��ȡ����ʧ��
    DEU_FAIL_GET_DB_AND_ID    = 206,    //  ��ȡdb��idʧ��
    DEU_FAIL_CREATE_DEUDB     = 207,    //  ����deudbʧ��
    DEU_FAIL_OPEN_DEUDB       = 208,    //  ��deudbʧ��
    DEU_FAIL_GET_POST_DATA    = 209,    //  ��ȡpost����ʧ��
    DEU_EMPTY_POST_DATA       = 210,    //  �յ�post����
    DEU_INVALID_POST_DATA     = 211,    //  ��Ч��post����
    DEU_FAIL_CONN_MYSQL       = 212,    //  �޷�����mysql
    DEU_FAIL_SELECT_DB        = 213,    //  ѡ��databaseʧ��
    DEU_FAIL_CREATE_DB        = 214,    //  ����database
    DEU_FAIL_CREATE_TABLE     = 215,    //    ������ʧ��
    DEU_FAIL_DELETE_DEUDB     = 216,    //  ɾ�����ݼ�ʧ��
    DEU_FAIL_DELETE_ATTR      = 217,    //  ɾ�����ݼ�����ʧ��
    DEU_FAIL_ADD_ATTR         = 218,    //  �������ʧ��
    DEU_FAIL_UPDATE_ATTR      = 219,    //  ��������ʧ��
    DEU_READ_EMPTY_DATA       = 220,    //  ��ȡ������Ϊ��
    DEU_FAIL_QUERY_ID         = 221,    //  �������Բ�IDʧ��
    DEU_FAIL_OPEN_FILE        = 222,    //  ���ļ�ʧ��
    DEU_FAIL_GET_PORT         = 223,    //  ��ȡ�˿�ʧ��
    DEU_FAIL_READ_CONF        = 224,    //  ��ȡ�����ļ�ʧ��
    DEU_EMPTY_QUERY_STRING    = 225,    //  �յ�����
    DEU_FAIL_ADD_BLOCK        = 226,    //  ������ݿ�ʧ�� 
    DEU_FAIL_ADD_VIRTUAL_TILE = 227,    //  ���������Ƭʧ��
    DEU_ALREADY_EXIST         = 228,    //  �����Ѿ�����
    DEU_FAIL_REPLACE_BLOCK    = 229,    //  �滻����ʧ��
    DEU_FAIL_GET_VERSION      = 230,    //  ��ȡ�汾ʧ��
    DEU_FAIL_ADD_CHILD        = 231,    //  ��Ӷ���IDʧ��
    DEU_PARENT_NOT_EXIST      = 232,    //  ��ͼ�㲻����
    DEU_CHILD_NOT_EXIST       = 233,    //  ��ͼ�㲻��
    DEU_NOT_CATEGORY_ID       = 234,    //  ���Ƿ��ſ�ID
    DEU_NOT_LAYER_ID          = 235,    //  ����ͼ��ID
    DEU_NO_ENOUGH_DISK_SPACE  = 236,    //  ����ռ�ʧ�ܣ������Ƿ���̿ռ䲻��

    DEU_FAIL_CALCU_EXTENT     = 301,    //  ������Ƭ��Χʧ��
    DEU_FAIL_INIT_INTERFACE   = 302,    //  ��ʼ���ӿ�ʧ��
    DEU_FAIL_GET_INTERFACE    = 303,    //  ��ȡ�ӿ�ʧ��
    DEU_FAIL_QUERY_OUT_DATA   = 304,    //  ��ȡ�ⲿ����ʧ��
    DEU_FAIL_SERVER_INFO      = 305,    //  �������ݼ������ȡ������Ϣʧ��
    DEU_FAIL_INIT_SERVER      = 306,    //  ��ʼ������ʧ��

    DEU_FAIL_USER_PWD         = 401,    //  ��ȡ�û���������ʧ��
    DEU_ERROR_PASSWD          = 402,    //  �û������������
    DEU_FAIL_MAKE_TICKET      = 403,    //  ����Ʊ��ʧ��
    DEU_FAIL_GET_ENTRY        = 404,    //  ��ȡ�û���Ŀ����
    DEU_INVALID_TICKET        = 405,    //  ��ЧƱ��,�����µ�¼
    DEU_NO_PERMISSION         = 406,    //  û����Ӧ��Ȩ��
    DEU_FAIL_GET_TICKET       = 407,    //  ��ȡƱ��ʧ��
    DEU_ALREADY_LOGIN         = 408,    //  �����û���¼���л��û����Ƚ���ǰ�û�ע��
    DEU_USER_NOT_LOGIN        = 409,    //  ���û�û�е�¼

    DEU_UNKNOWN               = 400     //  δ֪
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