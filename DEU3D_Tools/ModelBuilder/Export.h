// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� GLOBEBUILDER_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// GLOBEBUILDER_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�

#ifndef EXPORT_H_
#define EXPORT_H_

#ifdef WIN32
#ifdef MODELBUILDER_EXPORTS
    #define MB_EXPORTS    __declspec(dllexport)
#else
    #define MB_EXPORTS    __declspec(dllimport)
#endif
#else
    #define MB_EXPORTS
#endif

#endif
