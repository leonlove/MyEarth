#ifndef VARIANT_H_0D651E88_E3CC_4F2D_ACA9_4914BBD9B485_INCLUDE
#define VARIANT_H_0D651E88_E3CC_4F2D_ACA9_4914BBD9B485_INCLUDE

#include <string>
#include <vector>
#include <assert.h>
#include <IDProvider/ID.h>
#include <OpenSP/Ref.h>
#include <OpenSP/Observer.h>
#include <OpenSP/sp.h>
#include <OpenSP/op.h>
#include "Export.h"

namespace cmm
{

class CM_EXPORT variant_data
{
public:
    enum validate
    {
        VT_Null,
        VT_Bool,
        VT_Int,
        VT_UInt,
        VT_Int64,
        VT_UInt64,
        VT_Float,
        VT_Double,
        VT_String,
        VT_Pointer,
        VT_CPointer,
        VT_BoolList,
        VT_IntList,
        VT_UIntList,
        VT_Int64List,
        VT_UInt64List,
        VT_DoubleList,
        VT_FloatList,
        VT_StringList,
        VT_PointerList,
        VT_CPointerList,
        VT_ID,
        VT_IDList,
        VT_RefPointer,
        VT_CRefPointer,
        VT_RefPointerList,
        VT_CRefPointerList
    }m_eValidate;

    variant_data(void)                                      { m_eValidate = VT_Null;                                                                    }
    variant_data(const variant_data &var)                   { operator    =(var);                                                                       }
    variant_data(bool var)                                  { m_eValidate = VT_Bool;        m_bool          = var;                                      }
    variant_data(int var)                                   { m_eValidate = VT_Int;         m_int           = var;                                      }
    variant_data(unsigned int var)                          { m_eValidate = VT_UInt;        m_uint          = var;                                      }
    variant_data(__int64 var)                               { m_eValidate = VT_Int64;       m_int64         = var;                                      }
    variant_data(unsigned __int64 var)                      { m_eValidate = VT_UInt64;      m_uint64        = var;                                      }
    variant_data(float var)                                 { m_eValidate = VT_Float;       m_float         = var;                                      }
    variant_data(double var)                                { m_eValidate = VT_Double;      m_double        = var;                                      }
    variant_data(void *var)                                 { m_eValidate = VT_Pointer;     m_pComplexData  = var;                                      }
    variant_data(const void *var)                           { m_eValidate = VT_CPointer;    m_pComplexData  = (void *)var;                              }
    variant_data(const std::string &var)                    { m_eValidate = VT_String;      m_pComplexData  = new std::string(var);                     }
    variant_data(const std::vector<bool> &var)              { m_eValidate = VT_BoolList;    m_pComplexData  = new std::vector<bool>(var);               }
    variant_data(const std::vector<int> &var)               { m_eValidate = VT_IntList;     m_pComplexData  = new std::vector<int>(var);                }
    variant_data(const std::vector<unsigned int> &var)      { m_eValidate = VT_UIntList;    m_pComplexData  = new std::vector<unsigned int>(var);       }
    variant_data(const std::vector<__int64> &var)           { m_eValidate = VT_Int64List;   m_pComplexData  = new std::vector<__int64>(var);            }
    variant_data(const std::vector<unsigned __int64> &var)  { m_eValidate = VT_UInt64List;  m_pComplexData  = new std::vector<unsigned __int64>(var);   }
    variant_data(const std::vector<double> &var)            { m_eValidate = VT_DoubleList;  m_pComplexData  = new std::vector<double>(var);             }
    variant_data(const std::vector<std::string> &var)       { m_eValidate = VT_StringList;  m_pComplexData  = new std::vector<std::string>(var);        }
    variant_data(const std::vector<void *> &var)            { m_eValidate = VT_PointerList; m_pComplexData  = new std::vector<void *>(var);             }
    variant_data(const std::vector<const void *> &var)      { m_eValidate = VT_CPointerList;m_pComplexData  = new std::vector<const void *>(var);       }

    variant_data(const ID &var)                             { m_eValidate = VT_ID;          m_pComplexData  = new ID(var);                              }
    variant_data(const std::vector<ID> &var)                { m_eValidate = VT_IDList;      m_pComplexData  = new std::vector<ID>(var);                 }

    variant_data(const std::vector<float> &var)             { m_eValidate = VT_FloatList;   m_pComplexData  = new std::vector<float>(var);              }

    variant_data(OpenSP::Ref *var)                          { m_eValidate = VT_RefPointer;  m_pComplexData  = new OpenSP::sp<OpenSP::Ref>(var);         }
    variant_data(const OpenSP::Ref *var)                    { m_eValidate = VT_CRefPointer; m_pComplexData  = new OpenSP::sp<const OpenSP::Ref>(var);   }

    variant_data(const std::vector<OpenSP::sp<OpenSP::Ref> > &var)        { m_eValidate = VT_RefPointerList;  m_pComplexData = new std::vector<OpenSP::sp<OpenSP::Ref> >(var);          }
    variant_data(const std::vector<OpenSP::sp<const OpenSP::Ref> > &var)  { m_eValidate = VT_CRefPointerList; m_pComplexData = new std::vector<OpenSP::sp<const OpenSP::Ref> >(var);    }

    ~variant_data(void)
    {
        reset();
    }

    void reset(void)
    {
        switch(m_eValidate)
        {
            case VT_Null:               break;
            case VT_Bool:               break;
            case VT_Int:                break;
            case VT_UInt:               break;
            case VT_Int64:              break;
            case VT_UInt64:             break;
            case VT_Float:              break;
            case VT_Double:             break;
            case VT_Pointer:            break;
            case VT_CPointer:           break;
            case VT_String:             delete (std::string *)m_pComplexData;                                   break;
            case VT_ID:                 delete (ID *)m_pComplexData;                                            break;
            case VT_RefPointer:         delete (OpenSP::sp<OpenSP::Ref> *)m_pComplexData;                       break;
            case VT_CRefPointer:        delete (OpenSP::sp<OpenSP::Ref> *)m_pComplexData;                       break;
            case VT_BoolList:           delete (std::vector<bool>             *)m_pComplexData;                 break;
            case VT_IntList:            delete (std::vector<int>              *)m_pComplexData;                 break;
            case VT_UIntList:           delete (std::vector<unsigned int>     *)m_pComplexData;                 break;
            case VT_Int64List:          delete (std::vector<__int64>          *)m_pComplexData;                 break;
            case VT_UInt64List:         delete (std::vector<unsigned __int64> *)m_pComplexData;                 break;
            case VT_DoubleList:         delete (std::vector<double>           *)m_pComplexData;                 break;
            case VT_StringList:         delete (std::vector<std::string>      *)m_pComplexData;                 break;
            case VT_PointerList:        delete (std::vector<void *>           *)m_pComplexData;                 break;
            case VT_CPointerList:       delete (std::vector<const void *>     *)m_pComplexData;                 break;
            case VT_IDList:             delete (std::vector<ID>               *)m_pComplexData;                 break;
            case VT_FloatList:          delete (std::vector<float>            *)m_pComplexData;                 break;
            case VT_RefPointerList:     delete (std::vector<OpenSP::sp<OpenSP::Ref> >       *)m_pComplexData;   break;
            case VT_CRefPointerList:    delete (std::vector<OpenSP::sp<const OpenSP::Ref> > *)m_pComplexData;   break;
        }
    }

    const variant_data &operator=(const variant_data &var)
    {
        if(this == &var)    return *this;

        m_eValidate = var.m_eValidate;
        switch(m_eValidate)
        {
            case VT_Null:                                                   break;
            case VT_Bool:           m_bool         = var.m_bool;            break;
            case VT_Int:            m_int          = var.m_int;             break;
            case VT_UInt:           m_uint         = var.m_uint;            break;
            case VT_Int64:          m_int64        = var.m_int64;           break;
            case VT_UInt64:         m_uint64       = var.m_uint64;          break;
            case VT_Float:          m_float        = var.m_float;           break;
            case VT_Double:         m_double       = var.m_double;          break;
            case VT_Pointer:        m_pComplexData = var.m_pComplexData;    break;
            case VT_CPointer:       m_pComplexData = var.m_pComplexData;    break;
            case VT_RefPointer:     m_pComplexData = new OpenSP::sp<OpenSP::Ref>(var);          break;
            case VT_CRefPointer:    m_pComplexData = new OpenSP::sp<const OpenSP::Ref>(var);    break;
            case VT_ID:             m_pComplexData = new ID                                         ((ID)                           var);                   break;
            case VT_String:         m_pComplexData = new std::string                                ((std::string)                  var);                   break;
            case VT_BoolList:       m_pComplexData = new std::vector<bool>                          ((std::vector<bool>)            var);                   break;
            case VT_IntList:        m_pComplexData = new std::vector<int>                           ((std::vector<int>)             var);                   break;
            case VT_UIntList:       m_pComplexData = new std::vector<unsigned int>                  ((std::vector<unsigned int>)    var);                   break;
            case VT_Int64List:      m_pComplexData = new std::vector<__int64>                       ((std::vector<__int64>)         var);                   break;
            case VT_UInt64List:     m_pComplexData = new std::vector<unsigned __int64>              ((std::vector<unsigned __int64>)var);                   break;
            case VT_FloatList:      m_pComplexData = new std::vector<float>                         ((std::vector<float>)           var);                   break;
            case VT_DoubleList:     m_pComplexData = new std::vector<double>                        ((std::vector<double>)          var);                   break;
            case VT_StringList:     m_pComplexData = new std::vector<std::string>                   ((std::vector<std::string>)     var);                   break;
            case VT_PointerList:    m_pComplexData = new std::vector<void *>                        ((std::vector<void *>)          var);                   break;
            case VT_CPointerList:   m_pComplexData = new std::vector<const void *>                  ((std::vector<const void *>)    var);                   break;
            case VT_IDList:         m_pComplexData = new std::vector<ID>                            ((std::vector<ID>)              var);                   break;
            case VT_RefPointerList: m_pComplexData = new std::vector<OpenSP::sp<OpenSP::Ref> >      ((std::vector<OpenSP::sp<OpenSP::Ref> >)        var);   break;
        }
        return *this;
    }

    const variant_data &operator=(bool var)                                 { reset();  m_eValidate = VT_Bool;            m_bool         = var;             return *this; }
    const variant_data &operator=(int var)                                  { reset();  m_eValidate = VT_Int;             m_int          = var;             return *this; }
    const variant_data &operator=(unsigned int var)                         { reset();  m_eValidate = VT_UInt;            m_uint         = var;             return *this; }
    const variant_data &operator=(__int64 var)                              { reset();  m_eValidate = VT_Int64;           m_int64        = var;             return *this; }
    const variant_data &operator=(unsigned __int64 var)                     { reset();  m_eValidate = VT_UInt64;          m_uint64       = var;             return *this; }
    const variant_data &operator=(double var)                               { reset();  m_eValidate = VT_Double;          m_double       = var;             return *this; }
    const variant_data &operator=(float var)                                { reset();  m_eValidate = VT_Float;           m_float        = var;             return *this; }
    const variant_data &operator=(void *var)                                { reset();  m_eValidate = VT_Pointer;         m_pComplexData = var;             return *this; }
    const variant_data &operator=(const void *var)                          { reset();  m_eValidate = VT_CPointer;        m_pComplexData = (void *)var;     return *this; }
    const variant_data &operator=(const ID &var)                            { reset();  m_eValidate = VT_ID;              m_pComplexData = new ID(var);     return *this; }
    const variant_data &operator=(const std::string &var)                   { reset();  m_eValidate = VT_String;          m_pComplexData = new std::string(var);                    return *this; }
    const variant_data &operator=(const std::vector<bool> &var)             { reset();  m_eValidate = VT_BoolList;        m_pComplexData = new std::vector<bool>(var);              return *this; }
    const variant_data &operator=(const std::vector<int> &var)              { reset();  m_eValidate = VT_IntList;         m_pComplexData = new std::vector<int>(var);               return *this; }
    const variant_data &operator=(const std::vector<unsigned int> &var)     { reset();  m_eValidate = VT_UIntList;        m_pComplexData = new std::vector<unsigned int>(var);      return *this; }
    const variant_data &operator=(const std::vector<__int64> &var)          { reset();  m_eValidate = VT_Int64List;       m_pComplexData = new std::vector<__int64>(var);           return *this; }
    const variant_data &operator=(const std::vector<unsigned __int64> &var) { reset();  m_eValidate = VT_UInt64List;      m_pComplexData = new std::vector<unsigned __int64>(var);  return *this; }
    const variant_data &operator=(const std::vector<double> &var)           { reset();  m_eValidate = VT_DoubleList;      m_pComplexData = new std::vector<double>(var);            return *this; }
    const variant_data &operator=(const std::vector<std::string> &var)      { reset();  m_eValidate = VT_StringList;      m_pComplexData = new std::vector<std::string>(var);       return *this; }
    const variant_data &operator=(const std::vector<void *> &var)           { reset();  m_eValidate = VT_PointerList;     m_pComplexData = new std::vector<void *>(var);            return *this; }
    const variant_data &operator=(const std::vector<const void *> &var)     { reset();  m_eValidate = VT_CPointerList;    m_pComplexData = new std::vector<const void *>(var);      return *this; }

    const variant_data &operator=(const std::vector<ID> &var)               { reset();  m_eValidate = VT_IDList;          m_pComplexData = new std::vector<ID>(var);                return *this; }

    const variant_data &operator=(const std::vector<float> &var)            { reset();  m_eValidate = VT_FloatList;       m_pComplexData = new std::vector<float>(var);             return *this; }

    const variant_data &operator=(OpenSP::Ref *var)                         { reset();  m_eValidate = VT_RefPointer;      m_pComplexData = new OpenSP::sp<OpenSP::Ref>(var);        return *this; }
    const variant_data &operator=(const OpenSP::Ref *var)                   { reset();  m_eValidate = VT_CRefPointer;     m_pComplexData = new OpenSP::sp<const OpenSP::Ref>(var);  return *this; }

    const variant_data &operator=(const std::vector<OpenSP::sp<OpenSP::Ref> > &var)        { reset();  m_eValidate = VT_RefPointerList;  m_pComplexData = new std::vector<OpenSP::sp<OpenSP::Ref> >(var);         return *this; }
    const variant_data &operator=(const std::vector<OpenSP::sp<const OpenSP::Ref> > &var)  { reset();  m_eValidate = VT_CRefPointerList; m_pComplexData = new std::vector<OpenSP::sp<const OpenSP::Ref> >(var);   return *this; }

    operator bool             const &(void) const   { assert(m_eValidate == VT_Bool);     return m_bool;                        }
    operator bool                   &(void)         { assert(m_eValidate == VT_Bool);     return m_bool;                        }

    operator int              const &(void) const   { assert(m_eValidate == VT_Int);      return m_int;                         }
    operator int                    &(void)         { assert(m_eValidate == VT_Int);      return m_int;                         }

    operator unsigned int     const &(void) const   { assert(m_eValidate == VT_UInt);     return m_uint;                        }
    operator unsigned int           &(void)         { assert(m_eValidate == VT_UInt);     return m_uint;                        }

    operator __int64          const &(void) const   { assert(m_eValidate == VT_Int64);    return m_int64;                       }
    operator __int64                &(void)         { assert(m_eValidate == VT_Int64);    return m_int64;                       }

    operator unsigned __int64 const &(void) const   { assert(m_eValidate == VT_UInt64);   return m_uint64;                      }
    operator unsigned __int64       &(void)         { assert(m_eValidate == VT_UInt64);   return m_uint64;                      }

    operator float            const &(void) const   { assert(m_eValidate == VT_Float);    return m_float;                       }
    operator float                  &(void)         { assert(m_eValidate == VT_Float);    return m_float;                       }

    operator double           const &(void) const   { assert(m_eValidate == VT_Double);   return m_double;                      }
    operator double                 &(void)         { assert(m_eValidate == VT_Double);   return m_double;                      }

    operator       void*      const  (void) const   { assert(m_eValidate == VT_Pointer);  return m_pComplexData;                }
    operator       void*             (void)         { assert(m_eValidate == VT_Pointer);  return m_pComplexData;                }

    operator const void*      const  (void) const   { assert(m_eValidate == VT_CPointer); return m_pComplexData;                }
    operator const void*             (void)         { assert(m_eValidate == VT_CPointer); return m_pComplexData;                }

    operator ID               const &(void) const   { assert(m_eValidate == VT_ID);       return *(const ID *)m_pComplexData;   }
    operator ID                     &(void)         { assert(m_eValidate == VT_ID);       return *(      ID *)m_pComplexData;   }

    operator std::string      const &(void) const   { assert(m_eValidate == VT_String);   return *(const std::string *)(m_pComplexData);  }
    operator std::string            &(void)         { assert(m_eValidate == VT_String);   return *(      std::string *)(m_pComplexData);  }

    operator std::vector<bool>              const &(void) const { assert(m_eValidate == VT_BoolList);     return *(const std::vector<bool>              *)m_pComplexData;           }
    operator std::vector<bool>                    &(void)       { assert(m_eValidate == VT_BoolList);     return *(      std::vector<bool>              *)m_pComplexData;           }

    operator std::vector<int>               const &(void) const { assert(m_eValidate == VT_IntList);      return *(const std::vector<int>               *)m_pComplexData;           }
    operator std::vector<int>                     &(void)       { assert(m_eValidate == VT_IntList);      return *(      std::vector<int>               *)m_pComplexData;           }

    operator std::vector<unsigned int>      const &(void) const { assert(m_eValidate == VT_UIntList);     return *(const std::vector<unsigned int>      *)m_pComplexData;           }
    operator std::vector<unsigned int>            &(void)       { assert(m_eValidate == VT_UIntList);     return *(      std::vector<unsigned int>      *)m_pComplexData;           }

    operator std::vector<__int64>           const &(void) const { assert(m_eValidate == VT_Int64List);    return *(const std::vector<__int64>           *)m_pComplexData;           }
    operator std::vector<__int64>                 &(void)       { assert(m_eValidate == VT_Int64List);    return *(      std::vector<__int64>           *)m_pComplexData;           }

    operator std::vector<unsigned __int64>  const &(void) const { assert(m_eValidate == VT_UInt64List);   return *(const std::vector<unsigned __int64>  *)m_pComplexData;           }
    operator std::vector<unsigned __int64>        &(void)       { assert(m_eValidate == VT_UInt64List);   return *(      std::vector<unsigned __int64>  *)m_pComplexData;           }

    operator std::vector<double>            const &(void) const { assert(m_eValidate == VT_DoubleList);   return *(const std::vector<double>            *)m_pComplexData;           }
    operator std::vector<double>                  &(void)       { assert(m_eValidate == VT_DoubleList);   return *(      std::vector<double>            *)m_pComplexData;           }

    operator std::vector<std::string>       const &(void) const { assert(m_eValidate == VT_StringList);   return *(const std::vector<std::string>       *)m_pComplexData;           }
    operator std::vector<std::string>             &(void)       { assert(m_eValidate == VT_StringList);   return *(      std::vector<std::string>       *)m_pComplexData;           }

    operator std::vector<void *>            const &(void) const { assert(m_eValidate == VT_PointerList);  return *(const std::vector<void *>            *)m_pComplexData;           }
    operator std::vector<void *>                  &(void)       { assert(m_eValidate == VT_PointerList);  return *(      std::vector<void *>            *)m_pComplexData;           }

    operator std::vector<const void *>      const &(void) const { assert(m_eValidate == VT_CPointerList); return *(const std::vector<const void *>      *)m_pComplexData;           }
    operator std::vector<const void *>            &(void)       { assert(m_eValidate == VT_CPointerList); return *(      std::vector<const void *>      *)m_pComplexData;           }

    operator std::vector<ID>                const &(void) const { assert(m_eValidate == VT_IDList);       return *(const std::vector<ID>                *)m_pComplexData;           }
    operator std::vector<ID>                      &(void)       { assert(m_eValidate == VT_IDList);       return *(      std::vector<ID>                *)m_pComplexData;           }

    operator std::vector<float>             const &(void) const { assert(m_eValidate == VT_FloatList);    return *(const std::vector<float>             *)m_pComplexData;           }
    operator std::vector<float>                   &(void)       { assert(m_eValidate == VT_FloatList);    return *(      std::vector<float>             *)m_pComplexData;           }

    operator       OpenSP::Ref*             const  (void) const { assert(m_eValidate == VT_RefPointer);   return ((const OpenSP::sp<OpenSP::Ref>        *)m_pComplexData)->_ptr;    }
    operator       OpenSP::Ref*                    (void)       { assert(m_eValidate == VT_RefPointer);   return ((      OpenSP::sp<OpenSP::Ref>        *)m_pComplexData)->_ptr;    }

    operator const OpenSP::Ref*             const  (void) const { assert(m_eValidate == VT_CRefPointer);  return ((const OpenSP::sp<OpenSP::Ref>        *)m_pComplexData)->_ptr;    }
    operator const OpenSP::Ref*                    (void)       { assert(m_eValidate == VT_CRefPointer);  return ((      OpenSP::sp<OpenSP::Ref>        *)m_pComplexData)->_ptr;    }

    operator       std::vector<OpenSP::sp<OpenSP::Ref> >       const &(void) const { assert(m_eValidate == VT_RefPointerList);  return *(const std::vector<OpenSP::sp<OpenSP::Ref> >        *)m_pComplexData;   }
    operator       std::vector<OpenSP::sp<OpenSP::Ref> >             &(void)       { assert(m_eValidate == VT_RefPointerList);  return *(      std::vector<OpenSP::sp<OpenSP::Ref> >        *)m_pComplexData;   }

protected:
    union
    {
        bool                m_bool;
        int                 m_int;
        unsigned int        m_uint;
        __int64             m_int64;
        unsigned __int64    m_uint64;
        float               m_float;
        double              m_double;
        void*               m_pComplexData;
    };
};

typedef std::vector<variant_data> variant_list;

}

#endif

