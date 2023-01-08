{

     osFree Turbo Professional Copyright (C) 2022 osFree

     All rights reserved.

     Redistribution  and  use  in  source  and  binary  forms, with or without
modification, are permitted provided that the following conditions are met:

     *  Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
     *  Redistributions  in  binary  form  must  reproduce the above copyright
notice,   this  list  of  conditions  and  the  following  disclaimer  in  the
documentation and/or other materials provided with the distribution.
     * Neither the name of the osFree nor the names of its contributors may be
used  to  endorse  or  promote  products  derived  from  this software without
specific prior written permission.

     THIS  SOFTWARE  IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS"  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN  NO  EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES  (INCLUDING,  BUT  NOT  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES;  LOSS  OF  USE,  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

}

unit colordef;

interface

const
  BlackOnBlack       = $00;
  BlackOnBlue        = $10;
  BlackOnGreen       = $20;
  BlackOnCyan        = $30;
  BlackOnRed         = $40;
  BlackOnMagenta     = $50;
  BlackOnBrown       = $60;
  BlackOnLtGray      = $70;

  BlueOnBlack        = $01;
  BlueOnBlue         = $11;
  BlueOnGreen        = $21;
  BlueOnCyan         = $31;
  BlueOnRed          = $41;
  BlueOnMagenta      = $51;
  BlueOnBrown        = $61;
  BlueOnLtGray       = $71;
  
  GreenOnBlack       = $02;
  GreenOnBlue        = $12;
  GreenOnGreen       = $22;
  GreenOnCyan        = $32;
  GreenOnRed         = $42;
  GreenOnMagenta     = $52;
  GreenOnBrown       = $62;
  GreenOnLtGray      = $72;

  CyanOnBlack        = $03;
  CyanOnBlue         = $13;
  CyanOnGreen        = $23;
  CyanOnCyan         = $33;
  CyanOnRed          = $43;
  CyanOnMagenta      = $53;
  CyanOnBrown        = $63;
  CyanOnLtGray       = $73;

  RedOnBlack         = $04;
  RedOnBlue          = $14;
  RedOnGreen         = $24;
  RedOnCyan          = $34;
  RedOnRed           = $44;
  RedOnMagenta       = $54;
  RedOnBrown         = $64;
  RedOnLtGray        = $74;

  MagentaOnBlack     = $05;
  MagentaOnBlue      = $15;
  MagentaOnGreen     = $25;
  MagentaOnCyan      = $35;
  MagentaOnRed       = $45;
  MagentaOnMagenta   = $55;
  MagentaOnBrown     = $65;
  MagentaOnLtGray    = $75;

  BrownOnBlack       = $06;
  BrownOnBlue        = $16;
  BrownOnGreen       = $26;
  BrownOnCyan        = $36;
  BrownOnRed         = $46;
  BrownOnMagenta     = $56;
  BrownOnBrown       = $66;
  BrownOnLtGray      = $76;

  LtGrayOnBlack      = $07;
  LtGrayOnBlue       = $17;
  LtGrayOnGreen      = $27;
  LtGrayOnCyan       = $37;
  LtGrayOnRed        = $47;
  LtGrayOnMagenta    = $57;
  LtGrayOnBrown      = $67;
  LtGrayOnLtGray     = $77;

  DkGrayOnBlack      = $08;
  DkGrayOnBlue       = $18;
  DkGrayOnGreen      = $28;
  DkGrayOnCyan       = $38;
  DkGrayOnRed        = $48;
  DkGrayOnMagenta    = $58;
  DkGrayOnBrown      = $68;
  DkGrayOnLtGray     = $78;

  LtBlueOnBlack      = $09;
  LtBlueOnBlue       = $19;
  LtBlueOnGreen      = $29;
  LtBlueOnCyan       = $39;
  LtBlueOnRed        = $49;
  LtBlueOnMagenta    = $59;
  LtBlueOnBrown      = $69;
  LtBlueOnLtGray     = $79;

  LtGreenOnBlack     = $0A;
  LtGreenOnBlue      = $1A;
  LtGreenOnGreen     = $2A;
  LtGreenOnCyan      = $3A;
  LtGreenOnRed       = $4A;
  LtGreenOnMagenta   = $5A;
  LtGreenOnBrown     = $6A;
  LtGreenOnLtGray    = $7A;

  LtCyanOnBlack      = $0B;
  LtCyanOnBlue       = $1B;
  LtCyanOnGreen      = $2B;
  LtCyanOnCyan       = $3B;
  LtCyanOnRed        = $4B;
  LtCyanOnMagenta    = $5B;
  LtCyanOnBrown      = $6B;
  LtCyanOnLtGray     = $7B;

  LtRedOnBlack       = $0C;
  LtRedOnBlue        = $1C;
  LtRedOnGreen       = $2C;
  LtRedOnCyan        = $3C;
  LtRedOnRed         = $4C;
  LtRedOnMagenta     = $5C;
  LtRedOnBrown       = $6C;
  LtRedOnLtGray      = $7C;

  LtMagentaOnBlack   = $0D;
  LtMagentaOnBlue    = $1D;
  LtMagentaOnGreen   = $2D;
  LtMagentaOnCyan    = $3D;
  LtMagentaOnRed     = $4D;
  LtMagentaOnMagenta = $5D;
  LtMagentaOnBrown   = $6D;
  LtMagentaOnLtGray  = $7D;

  YellowOnBlack      = $0E;
  YellowOnBlue       = $1E;
  YellowOnGreen      = $2E;
  YellowOnCyan       = $3E;
  YellowOnRed        = $4E;
  YellowOnMagenta    = $5E;
  YellowOnBrown      = $6E;
  YellowOnLtGray     = $7E;

  WhiteOnBlack       = $0F;
  WhiteOnBlue        = $1F;
  WhiteOnGreen       = $2F;
  WhiteOnCyan        = $3F;
  WhiteOnRed         = $4F;
  WhiteOnMagenta     = $5F;
  WhiteOnBrown       = $6F;
  WhiteOnLtGray      = $7F;

implementation

begin
end.
