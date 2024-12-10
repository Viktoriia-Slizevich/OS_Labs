#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/time.h>
#include <linux/timekeeping.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module to create /proc/tsulab with time to Chinese New Year");

static struct proc_dir_entry *tsulab_entry;
static u64 previous_time_left = 0; // Предыдущее значение времени до китайского нового года

// Функция для вычисления времени до китайского нового года
static u64 time_to_chinese_new_year(void) 
{
    struct tm current_time, new_year_time;
    time64_t now, new_year;
    u64 diff;

    // Получаем текущее время
    now = ktime_get_real_seconds();
    time64_to_tm(now, 0, &current_time);

    // Устанавливаем время китайского нового года
    new_year_time.tm_year = current_time.tm_year + 1900 + 1; // Следующий год 
    new_year_time.tm_mon = 1; // febrary
    new_year_time.tm_mday = 10; // 10 date
    new_year_time.tm_hour = 0;
    new_year_time.tm_min = 0;
    new_year_time.tm_sec = 0;
    
    // Вычисляем время китайского нового года
    new_year = mktime64(new_year_time.tm_year, new_year_time.tm_mon + 1, new_year_time.tm_mday, new_year_time.tm_hour, new_year_time.tm_min, new_year_time.tm_sec);

    // Вычисляем разницу между текущим временем и временем нового года
    diff = new_year - now;

    // Преобразуем разницу в миллисекунды
    return diff * 1000;
}

// Функция для отображения информации в /proc/tsulab
static int tsulab_show(struct seq_file *m, void *v)
{
    u64 current_time_left = time_to_chinese_new_year();
    u64 time_passed = 0;

    if (previous_time_left > 0) {
        time_passed = previous_time_left - current_time_left;
    }

    seq_printf(m, "%llu\n", time_passed);

    // Сохраняем текущее значение для следующего открытия
    previous_time_left = current_time_left;

    return 0;
}

static int tsulab_open(struct inode *inode, struct file *file) //автоматом вызывается при команде в терминале для открытия файла
{
    return single_open(file, tsulab_show, NULL);
}

static const struct file_operations tsulab_fops = {
    .owner = THIS_MODULE,
    .open = tsulab_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

static int __init tsu_init(void)
{
    pr_info("HI\n");

    // Создаем файл /proc/tsulab
    tsulab_entry = proc_create("tsulab", 0, NULL, &tsulab_fops);
    if (!tsulab_entry) {
        pr_err("Failed to create /proc/tsulab\n");
        return -ENOMEM;
    }

    return 0;
}

static void __exit tsu_exit(void)
{
    pr_info("BYE\n");

    // Удаляем файл /proc/tsulab
    if (tsulab_entry) {
        remove_proc_entry("tsulab", NULL);
    }
}

module_init(tsu_init);
module_exit(tsu_exit);
