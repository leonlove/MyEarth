#include "export.h"
#include "IDProvider/ID.h"


namespace DK{

	class DATAKEEPER_API  IRecord
	{
	protected:
		enum METHOD {
			ADD,
			MODIFY
		};

	public:
		virtual void Vertify()				  = 0;
		virtual ID GetID()					  = 0;
		virtual std::string GetName()		  = 0;
		virtual void ShowInGlobe()			  = 0;
		virtual void GetObj()				  = 0;
		virtual void SetModify(METHOD Type)	  = 0;
		virtual void CommitModify()			  = 0;
	};


	class DATAKEEPER_API IRecordSet
	{
	public:
		virtual void Add(void* IR)	 = 0;
		virtual void Del(void* IR)   = 0;
		virtual void Vertify()		 = 0;
		virtual void Commit()		 = 0;
	};


	class DATAKEEPER_API IDataSet
	{
	public:
		virtual unsigned GetRecordCount() = 0;
		virtual unsigned GetRecords()	  = 0;
		virtual void Query()			  = 0;
		virtual void AddRecords(void* Rst) = 0;
		virtual void DelRecords(void* Rst) = 0;
		virtual void ReplaceRecords()	  = 0;
		virtual void Clear()			  = 0;
	};


	class IServer
	{
	public:
		virtual bool Init(const char* szIP) = 0;
		virtual bool FetchInfo()			= 0;
		virtual std::string GetInfo()		= 0;
	};

	class DATAKEEPER_API IDataServer : public IServer
	{
	public:
		virtual void Backup()  = 0;
		virtual void Restore() = 0;
	};

	class DATAKEEPER_API IRootServer : public IServer
	{
	public:
		virtual void AddDataServer()				= 0;
		virtual void RemoveDataServer()				= 0;
		virtual unsigned long GetDataServerCount()	= 0;
		virtual void Query()						= 0;
		virtual unsigned GetDatasetCount() const	= 0;
		virtual void GetDataset()					= 0;
		virtual	void DelDataset()					= 0;
	};


}