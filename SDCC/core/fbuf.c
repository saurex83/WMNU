/*!
\file Модуль реализует связанные буфера данных
\brief 
*/

#include "fbuf.h"
#include "string.h"
#include "mem.h"
#include "Net_frames.h"
#include "nwdebuger.h"

// Макросы для выравнивания памяти
// На текущий момент выравниевание не используется, так как были ошибки
// в работе макросов.
typedef uint16_t mem_ptr_t;
#define MEM_ALIGNMENT 2 //!< Кратность в байтах для выравнивания памяти
#define MEM_ALIGN_SIZE(size) (((size) + MEM_ALIGNMENT - 1) & ~(MEM_ALIGNMENT-1))
#define MEM_ALIGN(addr) ((void *)(((mem_ptr_t)(addr) + MEM_ALIGNMENT - 1) & ~(mem_ptr_t)(MEM_ALIGNMENT-1)))

// Доступные методы
fbuf_s* fbuf_create(uint8_t type, void* payload, uint8_t len);
void fbuf_delete(fbuf_s *fb);
void fbuf_chain(fbuf_s *h, fbuf_s *t);
fbuf_s* fbuf_next(fbuf_s *fb);
uint8_t fbuf_getCount(void);

// Локальные переменные модуля
static uint8_t NBR_BUF = 0; //!< Количество буферов в памяти


/**
@brief Создает fbuf указанного типа
@details 
@param type
@param payload
@param len
*/
fbuf_s* fbuf_create(uint8_t type, void* payload, uint8_t len)
{
  uint8_t req_len;
  // Определим длинну данных fbuf
  switch (type)
  {
    case FB_ETH_LAY:
      req_len = ETH_LAY_SIZE;
      break;
    case FB_IP_LAY:
      req_len = IP_LAY_SIZE;
      break;
    case FB_SYNC_LAY:
      req_len = SYNC_LAY_SIZE;
      break;
    case FB_TRANSPORT_LAY:
    case FB_RAW_LAY:
      req_len = len;
      break;
  default:
    ASSERT_HALT(false, "Incorrect fbuf type");
  } 
  
  // Выделяем память под структуру и данные и выравниваем в памяти
  // Выравнивание с помощью макросов приводило к пиздецу.
  fbuf_s* fb = (fbuf_s*)re_malloc(FBUF_S_SIZE + req_len);
  ASSERT_HALT(fb != NULL, "No memory");
  
  memset(fb, 0x77 , FBUF_S_SIZE + req_len);
  fb->payload = (uint8_t*)fb + FBUF_S_SIZE;
  
  // Заполним структуру
  fb->type = type;
  fb->next = NULL;
  fb->len = req_len;
  
  // Если пользователю нужно разместить сырые данные или транспортный
  // уровень, то скопируем данные
  if ((type == FB_TRANSPORT_LAY) || (type == FB_RAW_LAY))
    memcpy(fb->payload, payload, req_len);
  
  NBR_BUF++;
  return fb;
}

/**
@brief Получить количество буферов в памяти
@return Возвращает количество буферов
*/
uint8_t fbuf_getCount(void)
{
  return NBR_BUF;
}

/**
@brief Удаляем буфер
*/
void fbuf_delete(fbuf_s *fb)
{
  re_free(fb);
  NBR_BUF--;
}

/**
@brief Переход к следующему буферу
@return Указатель на следующий буфер или NULL
*/
fbuf_s* fbuf_next(fbuf_s *fb)
{
  if (fb->next == NULL)
    return NULL;
  return  fb->next;
}

/**
@brief Связывает два буфера. Head->Tail
@param[in,out] h head
@param[in] t tail
*/
void fbuf_chain(fbuf_s *h, fbuf_s *t)
{
  h->next = t;
}