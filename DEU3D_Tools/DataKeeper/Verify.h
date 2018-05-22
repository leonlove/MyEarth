#pragma once


namespace DK{

    class CVerify
    {
    public:
        CVerify(void);
        ~CVerify(void);

    public:

        bool Check();

    private:
        bool CheckVirtualTile();

        bool CheckTerrianTile();
        bool CheckTerrianTileHeightField();
        bool CheckTerrianTileImage();

        bool CheckModel();
        bool CheckModelProp();

        bool CheckImage();
        bool CheckImageProp();

        bool CheckShareModel();
        bool CheckShareModelProp();

        bool CheckLogicalLayer();
        bool CheckLogicalLayerProp();

        bool CheckTerrian();
        bool CheckTerrianProp();

        bool CheckTerrianDEM();
        bool CheckTerrianDEMProp();

        bool CheckTerrianDOM();
        bool CheckTerrianDOMProp();

        bool CheckTerrianLayer();
        bool CheckTerrianLayerProp();

        bool CheckTerrianDEMLayer();
        bool CheckTerrianDEMLayerProp();
        
        bool CheckTerrianDOMLayer();
        bool CheckTerrianDOMLayerProp();
        
        bool CheckLabelPoint();
        bool CheckLabelPointProp();

        bool CheckLabelLine();
        bool CheckLabelLineProp();

        bool CheckLabelFace();
        bool CheckLabelFaceProp();
    };

}