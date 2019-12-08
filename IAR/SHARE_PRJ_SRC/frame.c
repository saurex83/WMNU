/**
@file Модуль описывает сетевой кадр и связанные с ним буферы данных
@brief Типы frame
*/

#include "frame.h"
#include "string.h"
#include "mem.h"
#include "nwdebuger.h"
#include "ioCC2530.h"

frame_s* frame_create(void);
void frame_delete(frame_s *fr);
uint8_t frame_len(frame_s *fr);
uint8_t frame_getCount(void);

static uint8_t NBR_FRAME = 0; //!< Количество фреймов в памяти

/**
@brief Получить количество фреймов в памяти
@return Возвращает количество фреймов
*/
uint8_t frame_getCount(void)
{
  return NBR_FRAME;
}

/**
@brief Добавить заголовок
@param[in] fr указатель на фрейм
@param[in] src данные для добавления
@param[in] len размер данных
*/
void frame_addHeader(frame_s *fr, void *src, uint8_t len)
{
  unsigned short EA_save = EA;
  EA = 0; 
  // Ранее небыло создано данных 
  if (fr->payload == NULL)
    {
      fr->payload = re_malloc(len);
      fr->len = len;
      re_memcpy(fr->payload, src, len);
      EA = EA_save;
      return;
    }
  
  uint8_t new_len = fr->len + len; // Новый размер
  void *new_payload = re_malloc(new_len);
  ASSERT(new_payload != NULL);
  
  // Копируем данные в конец области
  re_memcpy((char*)new_payload + len, fr->payload, fr->len);
  // Копируем данные спереди в свободную область
  re_memcpy(new_payload, src, len);
  // Уничтожаем старые данные
  re_free(fr->payload);
  fr->payload = new_payload;
  fr->len = new_len;
  EA = EA_save;
}

/**
@brief Удалить заголовок
@param[in] fr указатель на фрейм
@param[in] len размер  удаляемых данных
*/
void frame_delHeader(frame_s *fr, uint8_t len)
{
  if (fr->payload == NULL)
    return;

  if (fr->len <= len)
    return;  
  
  uint8_t new_len = fr->len - len; // Новый размер
  void *new_payload = re_malloc(new_len);
  ASSERT(new_payload != NULL);
  
  // Копируем данные с пропуском первых len байт
  re_memcpy((char*)new_payload, (char*)fr->payload + len, new_len);
  
  // Уничтожаем старые данные
  re_free(fr->payload);
  fr->payload = new_payload;
  fr->len = new_len;
}

/**
@brief Создание структуры frame
@return Указатель на структуру frame
*/
frame_s* frame_create(void)
{
    unsigned short EA_save = EA;
    EA = 0; 
    
    frame_s* fr= (frame_s*)re_malloc(FRAME_S_SIZE);
    ASSERT(fr != NULL);
    
    fr->payload = NULL;
    fr->len = 0;
    memset(&fr->meta, 0x00, META_S_SIZE);
    NBR_FRAME++;
    
    EA = EA_save;
    return fr;
};

/**
@brief Удаление frame
@detail Проуедура так же удаляет все связанные с ней буфера fbuf
@param[in] fr указатель на структуру frame
*/
void frame_delete(frame_s *fr)
{
  unsigned short EA_save = EA;
  EA = 0;  
  
  if (fr->payload != NULL)
    re_free(fr->payload);
 
  NBR_FRAME--;
  re_free(fr);
  EA = EA_save;
}


/**
@brief Вычисляет размер цепочки fbuf
@param[in] fr указатель на frame_s
@return Количество байт в цепочке буферов
*/
uint8_t frame_len(frame_s *fr)
{
  return fr->len;
}