// Подключение необходимых заголовочных файлов
#include <linux/module.h>  // Для работы с модулями ядра
#include <linux/kernel.h>  // Для использования функций ядра (pr_info, printk)
#include <linux/init.h>    // Для макросов __init и __exit
#include <linux/proc_fs.h> // Для работы с файловой системой /proc
#include <linux/uaccess.h> // Для копирования данных между пространствами пользователя и ядра
#include <linux/time.h>    // Для работы со временем
#include <linux/timekeeping.h> // Для получения текущего времени

// Макросы для лицензии и автора модуля
MODULE_LICENSE("GPL");  // Лицензия модуля (General Public License)
MODULE_AUTHOR("Tomsk State University");  // Автор модуля
MODULE_DESCRIPTION("Module to create /proc/tsulab with time to Chinese New Year");  // Краткое описание модуля

// Определяем имя файла в каталоге /proc
#define PROC_FILE_NAME "tsulab"

// Переменные для хранения времени до китайского нового года
static u64 previous_time_left = 0; // Предыдущее значение времени до китайского нового года

// Указатель на запись в /proc
static struct proc_dir_entry *proc_file;

// Функция для вычисления времени до китайского нового года
static u64 time_to_chinese_new_year(void) {
    struct tm current_time, new_year_time;
    time64_t now, new_year;
    u64 diff;

    // Получаем текущее время
    now = ktime_get_real_seconds();
    time64_to_tm(now, 0, &current_time);

    // Устанавливаем время китайского нового года
    new_year_time.tm_year = current_time.tm_year + 1900 + 1; // Следующий год
    new_year_time.tm_mon = 1; // Февраль
    new_year_time.tm_mday = 10; // 10 февраля
    new_year_time.tm_hour = 0;
    new_year_time.tm_min = 0;
    new_year_time.tm_sec = 0;

    // Вычисляем время китайского нового года
    new_year = mktime64(new_year_time.tm_year, new_year_time.tm_mon + 1, new_year_time.tm_mday,
                        new_year_time.tm_hour, new_year_time.tm_min, new_year_time.tm_sec);

    // Вычисляем разницу между текущим временем и временем нового года
    diff = new_year - now;

    // Преобразуем разницу в миллисекунды
    return diff * 1000;
}

// Функция чтения из файла в /proc
static ssize_t tsulab_read(struct file *file, char __user *buf, size_t count, loff_t *pos) {
    char message[256]; // Буфер для сообщения
    int len;
    u64 current_time_left = time_to_chinese_new_year(); // Вычисляем текущее время до нового года
    u64 time_passed = 0;

    // Проверка позиции для EOF
    if (*pos > 0) {
        return 0; // EOF
    }

    // Вычисляем время, прошедшее с момента предыдущего чтения
    if (previous_time_left > 0) {
        time_passed = previous_time_left - current_time_left;
    }

    // Формируем строку с текущими данными
    len = snprintf(message, sizeof(message), "Время до китайского нового года: %llu мс, прошло времени: %llu мс\n", current_time_left, time_passed);

    // Сохраняем текущее значение для следующего чтения
    previous_time_left = current_time_left;

    // Копируем данные из пространства ядра в пространство пользователя
    if (copy_to_user(buf, message, len)) {
        return -EFAULT; // Возвращаем ошибку, если копирование не удалось
    }

    // Обновляем позицию для следующего чтения
    *pos += len;
    return len;
}

// Операции для работы с файлом в /proc
static const struct proc_ops proc_file_ops = {
    .proc_read = tsulab_read,
};

// Функция инициализации модуля
// Выполняется при загрузке модуля в ядро
static int __init tsu_module_init(void) {
    pr_info("Welcome to the Tomsk State University\n");  // Вывод сообщения в журнал ядра

    // Создаём файл в /proc
    proc_file = proc_create(PROC_FILE_NAME, 0444, NULL, &proc_file_ops);
    if (!proc_file) {
        pr_err("Failed to create /proc/%s\n", PROC_FILE_NAME);
        return -ENOMEM;
    }

    pr_info("/proc/%s created\n", PROC_FILE_NAME);
    return 0;  // Возвращаем 0 при успешной инициализации
}

// Функция очистки модуля
// Выполняется при выгрузке модуля из ядра
static void __exit tsu_module_exit(void) {
    // Удаляем файл из /proc
    proc_remove(proc_file);
    pr_info("/proc/%s removed\n", PROC_FILE_NAME);

    pr_info("Tomsk State University forever!\n");  // Вывод сообщения в журнал ядра
}

// Макросы для указания функций инициализации и очистки
module_init(tsu_module_init);  // Указание функции инициализации
module_exit(tsu_module_exit);  // Указание функции очистки
