#pragma once

#include "Haruoto.h"

class CWindRP : public CHaruoto
{
public:

    virtual BOOL Mount( CArcFile* pclArc );
    virtual BOOL Decode( CArcFile* pclArc );


protected:

    virtual void InitMountKey( CArcFile* pclArc );
    virtual void InitDecodeKey( CArcFile* pclArc );
};
