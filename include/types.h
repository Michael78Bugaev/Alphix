// Alphix kernel
// Basic types for Alphix
// include/types.h

#ifndef _ALPHIX_TYPES_H
#define _ALPHIX_TYPES_H

// Стандартные целочисленные типы
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed int         int32_t;
typedef signed long long   int64_t;

// Типы размеров
typedef uint64_t size_t;
typedef int64_t  ssize_t;
typedef int64_t  off_t;

// POSIX типы
typedef int32_t  pid_t;     // ID процесса
typedef uint32_t uid_t;     // ID пользователя
typedef uint32_t gid_t;     // ID группы
typedef uint32_t mode_t;    // Режим файла
typedef uint64_t ino_t;     // Номер inode
typedef uint32_t dev_t;     // ID устройства
typedef uint32_t nlink_t;   // Количество ссылок
typedef uint64_t time_t;    // Время

// Указатели
typedef uint64_t uintptr_t;
typedef int64_t  intptr_t;

// Логический тип
typedef _Bool bool;
#define true  1
#define false 0

// NULL указатель
#ifndef NULL
#define NULL ((void*)0)
#endif

#endif // _ALPHIX_TYPES_H 