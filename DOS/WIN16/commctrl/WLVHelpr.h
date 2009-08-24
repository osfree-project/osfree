
void 
LVCols_Init ( PLVIEWCOLS      );

void LVCols_Destroy ( PLVIEWCOLS      );

int LVCols_InsertCol ( PLVIEWCOLS, int, LV_COLUMN*);

BOOL LVCols_LocateSubItem ( PLVIEWCOL, PLVITEMPATH, BOOL);

void LVRows_Init ( PLVIEWROWS      );

int LVRows_SetRowItm ( PLVIEWROWS, int, LV_ITEM*, UINT);

