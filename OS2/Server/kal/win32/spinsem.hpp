class SpinMutexSemaphore {
        LONG toggle;
        LONG waiters;
        HANDLE try_again;
        HANDLE current_owner;
        unsigned own_count;
public:
        int Initialize() {
          toggle  = 0;
          waiters = 0;
          current_owner = 0;
          own_count = 0;
          try_again = CreateEvent(NULL, FALSE, FALSE, NULL);
          return try_again!=INVALID_HANDLE_VALUE;
        }
        Finalize() {
          CloseHandle(try_again);
        }
        void Request();
        void Release();
};

