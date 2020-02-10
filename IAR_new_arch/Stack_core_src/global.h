//C compiler options->extra options add  --no_path_in_file_macros

#define DEBUG
#define PRINT_TO_TERMINAL
#define SIG_DEBUG


#define FRAME_FOOTER_DEL //!< FR_del_header заполняет пустое место нулями
#define FILL_SLOT_ZERO //!< SL_alloc заполняет буфер нулями

#define MAX_PACKETS 20 
#define SLOT_POOL_ITEMS MAX_PACKETS //!< Количество слотов памяти в системе
#define FRAME_POOL_ITEMS_COUNT MAX_PACKETS //!< Количество фреймов в системе
#define MAX_TXRX_BUFFER MAX_PACKETS //!< Размер буфера