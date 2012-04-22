#pragma once

#include "HaruotoFD.h"

class Cef_trial : public CHaruotoFD
{
public:

    virtual BOOL Mount( CArcFile* pclArc );
    virtual BOOL Decode( CArcFile* pclArc );


protected:

    virtual void InitMountKey( CArcFile* pclArc );
    virtual void InitDecodeKey( CArcFile* pclArc );
};
