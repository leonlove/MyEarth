// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� TILEBUILDER_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// TILEBUILDER_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef TILEBUILDER_EXPORTS
#define TILEBUILDER_API __declspec(dllexport)
#else
#define TILEBUILDER_API __declspec(dllimport)
#endif

// �����Ǵ� TileBuilder.dll ������
class TILEBUILDER_API CTileBuilder {
public:
    CTileBuilder(void);
    // TODO: �ڴ�������ķ�����
};

extern TILEBUILDER_API int nTileBuilder;

TILEBUILDER_API int fnTileBuilder(void);
