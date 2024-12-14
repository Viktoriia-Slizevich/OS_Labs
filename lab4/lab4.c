
#include <linux/module.h> 
#include <linux/kernel.h>  
#include <linux/init.h>    
#include <linux/proc_fs.h> 
#include <linux/uaccess.h> // Для копирования данных между пространствами пользователя и ядра
#include <linux/time.h>   
#include <linux/timekeeping.h> 

MODULE_LICENSE("GPL");  
MODULE_DESCRIPTION("Module to create /proc/tsulab with time to Chinese New Year");  // Краткое описание модуля

#define PROC_FILE_NAME "tsulab"
static u64 previous_time_left = 0; // Предыдущее значение времени до китайского нового года

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
    diff = new_year - now;
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
        return 0; 
    }

    // Вычисляем время, прошедшее с момента предыдущего чтения
    if (previous_time_left > 0) {
        time_passed = previous_time_left - current_time_left;
    }

    // Формируем строку с данными
    len = snprintf(message, sizeof(message), "Время до китайского нового года: %llu мс, прошло времени с предыдущего обращения: %llu мс\n", current_time_left, time_passed);

    // Сохраняем текущее значение для следующего чтения
    previous_time_left = current_time_left;

    // Копируем данные из пространства ядра в пространство пользователя
    if (copy_to_user(buf, message, len)) {
        return -EFAULT; 
    }

    // Обновляем позицию для следующего чтения
    *pos += len;
    return len;
}
// надо для работы с proc
static const struct file_operations proc_file_fops = {
    .owner = THIS_MODULE,
    .read = tsulab_read,
};

static int __init tsu_module_init(void) {
    pr_info("HI\n");  // Вывод сообщения в журнал ядра

    proc_file = proc_create(PROC_FILE_NAME, 0444, NULL, &proc_file_fops);
    if (!proc_file) {
        pr_err("Failed to create /proc/%s\n", PROC_FILE_NAME);
        return -ENOMEM;
    }

    pr_info("/proc/%s created\n", PROC_FILE_NAME);
    return 0;  // успех
}

static void __exit tsu_module_exit(void) {
    proc_remove(proc_file);
    pr_info("/proc/%s removed\n", PROC_FILE_NAME);

    pr_info("BYE!\n");  
}

module_init(tsu_module_init);  
module_exit(tsu_module_exit);  
