/**
@file Модуль описывает сетевой кадр и связанные с ним буферы данных
@brief Типы frame
*/

#include "frame.h"
#include "string.h"
#include "stdlib.h"
#include "Net_frames.h"
#include "nwdebuger.h"

frame_s* frame_create(void);
void frame_delete(frame_s *fr);
void frame_insert_head(frame_s *fr , fbuf_s *fb);
void frame_insert_tail(frame_s *fr , fbuf_s *fb);
fbuf_s* frame_get_fbuf_head(frame_s *fr);
fbuf_s* frame_get_fbuf_tail(frame_s *fr);
void* frame_merge(frame_s *fr, uint8_t *len);
uint8_t frame_len(frame_s *fr);

/**
@brief Создание структуры frame
@return Указатель на структуру frame
*/
frame_s* frame_create(void)
{
    frame_s* fr= (frame_s*)malloc(FRAME_S_SIZE);
    ASSERT_HALT(fr != NULL, "No memory");
    
    fr->head = NULL;
    fr->tail = NULL;
    memset(&fr->meta, 0x00, META_S_SIZE);
    return fr;
};

/**
@brief Удаление frame
@detail Проуедура так же удаляет все связанные с ней буфера fbuf
@param[in] fr указатель на структуру frame
*/
void frame_delete(frame_s *fr)
{
  fbuf_s *fb = fr->head;
  fbuf_s *next = fb;
  
  // Удаляем все связанные буфера
  while (fb != NULL)
  {
      next = fb->next;
      free(fb);
      fb = next;
  };
  
  free(fr);
}

/**
@brief Вставить буфер в начало цепочки
@param[in,out] fr указатель на структуру frame
@param[in] fb указатель на буфер для вставки
*/
void frame_insert_head(frame_s *fr , fbuf_s *fb)
{
  if (fr->head == NULL)
  {
    fr->head = fb;
    fr->tail = fb;
    return;
  }
  
  fb->next = fr->head; 
  fr->head = fb;
}

/**
@brief Вставить буфер в конец цепочки
@param[in,out] fr указатель на структуру frame
@param[in] fb указатель на буфер для вставки
*/
void frame_insert_tail(frame_s *fr , fbuf_s *fb)
{
  ASSERT_HALT(fr != NULL, "No fr");
  ASSERT_HALT(fb != NULL, "No fb");
  
  if (fr->tail == NULL)
  {
    fr->head = fb;
    fr->tail = fb;
    return;
  }
  
  fr->tail->next = fb;
  fr->tail = fb;
}

/**
@brief Получить указатель на первый элемент в цепочке
@param[in] fr указатель на структуру frame
@return указатель на первый буфер или NULL
*/
fbuf_s* frame_get_fbuf_head(frame_s *fr)
{
  return fr->head;
}

/**
@brief Получить указатель на последний элемент в цепочке
@param[in] fr указатель на структуру frame
@return указатель на последний буфер или NULL
*/
fbuf_s* frame_get_fbuf_tail(frame_s *fr)
{
  return fr->tail;
}

/**
@brief Производит слияние всех данных в один массив
@details Данные требуют удаления после использования free(..)
@param[in] fr указатель на frame
@param[out] len длинна результирующего массива данных
@return Указатель на начало данных
*/
void* frame_merge(frame_s *fr, uint8_t *len)
{
  ASSERT_HALT(fr != NULL, "No fr");
  
  uint8_t tot_len = frame_len(fr); // Общая длинна данных
  *len = tot_len;
  
  // Выходим если данных нет
  if (!tot_len) 
    return NULL;
  
  // Выделяем память
  void* mem = malloc(tot_len);
  ASSERT_HALT(mem != NULL, "No memory");
  
  uint8_t *ptr = (uint8_t*)mem;
  
  // Копируем данные
  fbuf_s *fb = fr->head;
  while (fb!= NULL)
  {
    memcpy(ptr, fb->payload, fb->len);
    ptr += fb->len;
    fb = fb->next;
  }
  return mem;
}

/**
@brief Вычисляет размер цепочки fbuf
@param[in] fr указатель на framr
@return Количество байт в цепочке буферов
*/
uint8_t frame_len(frame_s *fr)
{
  uint8_t tot_len = 0;
  fbuf_s *fb = fr->head;
 
  while (fb != NULL)
  {
    tot_len += fb->len;
    fb = fb->next;
  }
  return tot_len;
}