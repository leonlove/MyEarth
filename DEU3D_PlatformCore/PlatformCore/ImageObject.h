#ifndef IMAGE_OBJECT_H_C27298A7_F29D_4BF0_BEED_88381C65F2D9_INCLUDE
#define IMAGE_OBJECT_H_C27298A7_F29D_4BF0_BEED_88381C65F2D9_INCLUDE

#include "IImageObject.h"
#include <osg/ImageUtils>

#pragma pack(push, 1)
struct PixelData
{
    unsigned  LowBit : 4;
    unsigned  HighBit : 4;
};
#pragma pack(pop)

class ImageObject : public IImageObject
{
    friend class DEUSceneViewer;
    friend class FrameImageFetcher;

public:
    explicit ImageObject(void)
    {
        m_pImageInternal = new osg::Image;
    }
    virtual ~ImageObject(void)
    {
        m_pImageInternal = NULL;
    }
    virtual unsigned getWidth(void) const
    {
        if(!m_pImageInternal.valid())   return 0u;
        return m_pImageInternal->s();
    }
    virtual unsigned getHeight(void) const
    {
        if(!m_pImageInternal.valid())   return 0u;
        return m_pImageInternal->t();
    }
    virtual unsigned getImageSize(void) const
    {
        if(!m_pImageInternal.valid())   return 0u;
        return m_pImageInternal->getImageSizeInBytes();
    }

    virtual const void *getImageData(void) const
    {
        if(!m_pImageInternal.valid())   return NULL;
        return m_pImageInternal->data();
    }
    virtual void *getImageData(void)
    {
        if(!m_pImageInternal.valid())   return NULL;
        return m_pImageInternal->data();
    }
    
    virtual IImageObject *swapRedAndBlueChanel()
    {
        if(!m_pImageInternal.valid())   return false;
        
        osg::ref_ptr<osg::Image> clone = new osg::Image(*m_pImageInternal, osg::CopyOp::DEEP_COPY_ALL);
        unsigned ChanelCnt = 0;

        if(m_pImageInternal->getPixelFormat() == GL_RGBA) 
        {
            ChanelCnt = 4;
        }
        else if(m_pImageInternal->getPixelFormat() == GL_RGB) 
        {
            ChanelCnt = 3;
        }
        else return false;

        unsigned PixelInBytes       = m_pImageInternal->getPixelSizeInBits() / 8;
        unsigned PixelCnt           = m_pImageInternal->getImageSizeInBytes() / PixelInBytes;
        unsigned PixelChanelInBytes = PixelInBytes / ChanelCnt;
        unsigned OffsetFromB2R      = PixelChanelInBytes * 2;

        for (unsigned i = 0; i < PixelCnt; i++)
        {
            memcpy(&clone->data()[i * ChanelCnt], &m_pImageInternal->data()[i * ChanelCnt + OffsetFromB2R], PixelChanelInBytes);
            memcpy(&clone->data()[i * ChanelCnt + OffsetFromB2R], &m_pImageInternal->data()[i * ChanelCnt], PixelChanelInBytes);
        }

        ImageObject *tmp = new ImageObject;
        tmp->m_pImageInternal = clone;
        return tmp;
    }

    virtual bool flipVertical()
    {
        if(!m_pImageInternal.valid())   return false;

        m_pImageInternal->flipVertical();
        return true;

    }

    virtual bool setImage(unsigned w, unsigned h, const void *data, unsigned len)
    {
        if (w > 1 && h > 1 && len > 2 && data)
        {
            m_pImageInternal->setImage(w, h, 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char *)data, osg::Image::USE_NEW_DELETE);
            return true;
        }

        return false;
    }

    virtual bool  saveToFile(const std::string &filename) const
    {
        if(!m_pImageInternal.valid())   return false;

        return osgDB::writeImageFile(*m_pImageInternal, filename);
    }

    virtual bool scale(unsigned size)
    {
        if(!m_pImageInternal.valid())   return false;

        osg::ref_ptr<osg::Image> dst = new osg::Image;
        dst->allocateImage(size, size, m_pImageInternal->r(), m_pImageInternal->getPixelFormat(), m_pImageInternal->getDataType());
        osg::clearImageToColor(dst, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

        if (m_pImageInternal->s() > m_pImageInternal->t())
        {
            m_pImageInternal->scaleImage(size, m_pImageInternal->t() * size / m_pImageInternal->s(), 1);

            osg::copyImage(m_pImageInternal.get(), 
                0,0,0,
                m_pImageInternal->s(), m_pImageInternal->t(), m_pImageInternal->r(),
                dst.get(),
                0, (size - m_pImageInternal->t()) / 2, 0);
        }
        else
        {
            m_pImageInternal->scaleImage(m_pImageInternal->s() * size / m_pImageInternal->t(), size, 1);

            osg::copyImage(m_pImageInternal.get(), 
                0,0,0,
                m_pImageInternal->s(), m_pImageInternal->t(), m_pImageInternal->r(),
                dst.get(),
                (size - m_pImageInternal->s()) / 2, 0, 0);
        }
        
        m_pImageInternal = dst;
        return true;
    }
	virtual bool fromJson(bson::bsonElement &val)
	{
		if (val.GetType() != bson::bsonDocType)
		{
			return false;
		}

		bson::bsonDocumentEle *elem = dynamic_cast<bson::bsonDocumentEle*>(&val);
		bson::bsonDocument &doc = elem->GetDoc();

		if (doc.GetElement("Width") == NULL   || doc.GetElement("Width")->GetType() != bson::bsonDoubleType ||
			doc.GetElement("Height") == NULL  || doc.GetElement("Height")->GetType() != bson::bsonDoubleType ||
			doc.GetElement("DataLen") == NULL || doc.GetElement("DataLen")->GetType() != bson::bsonDoubleType ||
			doc.GetElement("Data") == NULL    || doc.GetElement("Data")->GetType() != bson::bsonStringType)
		{
			return false;
		}

		bson::bsonDoubleEle *w    = dynamic_cast<bson::bsonDoubleEle*>(doc.GetElement("Width"));
		bson::bsonDoubleEle *h    = dynamic_cast<bson::bsonDoubleEle*>(doc.GetElement("Height"));
		bson::bsonDoubleEle *l  = dynamic_cast<bson::bsonDoubleEle*>(doc.GetElement("DataLen"));
		bson::bsonStringEle *data = dynamic_cast<bson::bsonStringEle*>(doc.GetElement("Width"));

		const char* src_data = data->StrValue();
		unsigned src_len = (unsigned)l->DblValue();
		unsigned len = src_len / 2;

		if (src_len < 5 || src_len % 2 != 0 || src_data[src_len] != 0)
		{
			//编码后的图片大小必须大于等于2*2,并且最后一个字符是0结尾
			return false;
		}
    
		char *img_data = new char[len];
		char *tmp = img_data;

		for (unsigned i = 0; i < len ; i++)
		{
			PixelData *pTemp = (PixelData *)tmp;

			pTemp->LowBit = (0x0f & *src_data++);
			pTemp->HighBit = (0x0f & *src_data++);

			tmp++;
		}

		setImage((unsigned long)w->DblValue(), (unsigned long)h->DblValue(), img_data, len);

		return true;
	}

	virtual void toJson(bson::bsonElement &val)
	{
		bson::bsonDoubleEle   *elem_w     = NULL;
		bson::bsonDoubleEle   *elem_h     = NULL;
		bson::bsonDoubleEle   *elem_len   = NULL;
		bson::bsonStringEle   *elem_data  = NULL;
		bson::bsonDocumentEle *doc_elem   = dynamic_cast<bson::bsonDocumentEle*>(&val);

		for (unsigned int i = 0; i < doc_elem->GetDoc().ChildCount(); i++)
		{
			bson::bsonElement *e = doc_elem->GetDoc().GetElement(i);

			if (strcmp(e->EName(), "Width") == 0 && e->GetType() == bson::bsonDoubleType)
			{
				elem_w = dynamic_cast<bson::bsonDoubleEle*>(e);
				elem_w->SetDblValue(getWidth());
			}

			if (strcmp(e->EName(), "Height") == 0 && e->GetType() == bson::bsonDoubleType)
			{
				elem_h = dynamic_cast<bson::bsonDoubleEle*>(e);
				elem_h->SetDblValue(getHeight());
			}

			if (strcmp(e->EName(), "DataLen") == 0 && e->GetType() == bson::bsonDoubleType)
			{
				elem_h = dynamic_cast<bson::bsonDoubleEle*>(e);
			}

			if (strcmp(e->EName(), "Data") == 0 && e->GetType() == bson::bsonStringType)
			{
				elem_data = dynamic_cast<bson::bsonStringEle*>(e);
			}
		}

		const char *src = (const char*)getImageData();
		const unsigned src_len = getImageSize();

		unsigned int len = getImageSize() * 2 + 1;
		char *dst = new char[len];
		char *tmp = dst;

		for (unsigned i = 0; i <  src_len; i++)
		{
			PixelData *pTemp = (PixelData *)src;

			*tmp++ = pTemp->LowBit | 0x10;
			*tmp++ = pTemp->HighBit | 0x10;

			src++;
		}

		*tmp = 0;
    
		if (!elem_w)
		{
			doc_elem->GetDoc().AddDblElement("Width", getWidth());
		}

		if (!elem_h)
		{
			doc_elem->GetDoc().AddDblElement("Width", getHeight());
		}

		if (elem_len)
		{
			elem_len->SetDblValue(len);
		}
		else
		{
			doc_elem->GetDoc().AddDblElement("DataLen", len);
		}

		if (elem_data)
		{
			elem_data->SetStrValue(dst);
		}
		else
		{
			doc_elem->GetDoc().AddStringElement("Data", dst);
		}

		delete[] dst;
	}

protected:
public:
    osg::ref_ptr<osg::Image>    m_pImageInternal;
};


#endif
