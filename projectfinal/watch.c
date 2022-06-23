#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/sched/cputime.h>
#include <linux/mm.h>
#include <linux/mm_types.h>

#define MAX_SIZE 1024

static struct proc_dir_entry* proc_ent;
static char output[MAX_SIZE];
static int out_len;
static struct task_struct* taskp = NULL;

int test_and_clear_young(pte_t* ptep)
{
    int ret = 0;

    if (pte_young(*ptep))
        ret = test_and_clear_bit(_PAGE_BIT_ACCESSED,
            (unsigned long*)&ptep->pte);

    return ret;
}

static pte_t* get_pte_from_task(struct task_struct* task, unsigned long addr) {
    pgd_t* pgd;
    p4d_t* p4d;
    pud_t* pud;
    pmd_t* pmd;
    pte_t* pte;

    pgd = pgd_offset(task->mm, addr);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) return NULL;

    p4d = p4d_offset(pgd, addr);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) return NULL;

    pud = pud_offset(p4d, addr);
    if (pud_none(*pud) || pud_bad(*pud)) return NULL;

    pmd = pmd_offset(pud, addr);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) return NULL;

    pte = pte_offset_kernel(pmd, addr);
    if (pte_none(*pte)) return NULL;

    return pte;
}

static ssize_t proc_read(struct file* fp, char __user* ubuf, size_t len, loff_t* pos)
{
    int count; /* the number of characters to be copied */
    u64 utime, stime;
    int young;
    struct vm_area_struct* vma;
    pte_t* ptep;
    int sum = 0;
    unsigned long addr;


    if (*pos == 0 && taskp) {
        /* a new read, update process' status */
        // cpu usage
        out_len = 0;
        task_cputime_adjusted(taskp, &utime, &stime);

        out_len += sprintf(output + out_len, "%lld ", utime);
        out_len += sprintf(output + out_len, "%lld ", stime);
        out_len += sprintf(output + out_len, "\n");

        // memory

        for (vma = taskp->mm->mmap; vma != NULL; vma = vma->vm_next) {
            for (addr = vma->vm_start; addr < vma->vm_end; addr += 4096) {
                ptep = get_pte_from_task(taskp, addr);
                if (!ptep)continue;
                young = test_and_clear_young(ptep);
                sum += young;
            }
        }
        out_len += sprintf(output + out_len, "%d ", sum);
        out_len += sprintf(output + out_len, "\n");
    }

    if (out_len - *pos > len) {
        count = len;
    }
    else {
        count = out_len - *pos;
    }

    pr_info("Reading the proc file\n");
    if (copy_to_user(ubuf, output + *pos, count)) return -EFAULT;
    *pos += count;

    return count;
}

static ssize_t proc_write(struct file* fp, const char __user* ubuf, size_t len, loff_t* pos)
{
    int pid;

    if (*pos > 0) return -EFAULT;
    pr_info("Writing the proc file\n");
    if (kstrtoint_from_user(ubuf, len, 10, &pid)) return -EFAULT;

    taskp = get_pid_task(find_get_pid(pid), PIDTYPE_PID);

    *pos += len;
    return len;
}

static const struct proc_ops proc_ops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

static int __init watch_init(void)
{
    proc_ent = proc_create("watch", 0666, NULL, &proc_ops);
    if (!proc_ent) {
        proc_remove(proc_ent);
        pr_alert("Error: Could not initialize /proc/watch\n");
        return -EFAULT;
    }
    pr_info("/proc/watch created\n");
    return 0;
}

static void __exit watch_exit(void)
{
    proc_remove(proc_ent);
    pr_info("/proc/watch removed\n");
}

module_init(watch_init);
module_exit(watch_exit);
MODULE_LICENSE("GPL");