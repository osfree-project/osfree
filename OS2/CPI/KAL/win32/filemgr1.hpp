struct ntFILE {
        HANDLE ntFileHandle;
        ULONG mode;
};

extern HandleTable<ntFILE,5,50> FileTable;

