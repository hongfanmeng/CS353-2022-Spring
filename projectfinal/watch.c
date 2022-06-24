#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/sched/cputime.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/seq_file.h>  

#define MAX_SIZE 1024
#define MAX_PID_CNT 128

static char input[MAX_SIZE];
static struct proc_dir_entry* proc_ent;
static struct task_struct* taskp_list[MAX_PID_CNT];
static int pid_cnt = 0;

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

int show_stat(struct seq_file* m, void* v)
{
    u64 utime, stime, pte_count;
    int young, i;
    struct vm_area_struct* vma;
    pte_t* ptep;
    unsigned long addr;
    pid_t pid;
    struct task_struct* taskp;


    for (i = 0; i < pid_cnt; i++) {
        taskp = taskp_list[i];
        if (!taskp)continue;

        // print pid
        pid = taskp->pid;
        seq_printf(m, "%d ", pid);

        // task is exit
        if (taskp->exit_state & EXIT_TRACE) {
            pr_info("proc watch: task of pid %d is exit.\n", pid);
            seq_printf(m, "-1\n");
            return 0;
        }

        // cpu usage
        task_cputime_adjusted(taskp, &utime, &stime);

        // print utime and stime
        seq_printf(m, "%lld ", utime);
        seq_printf(m, "%lld ", stime);
        // seq_printf(m, "\n");

        // calc mem
        pte_count = 0;
        for (vma = taskp->mm->mmap; vma != NULL; vma = vma->vm_next) {
            for (addr = vma->vm_start; addr < vma->vm_end; addr += 4096) {
                ptep = get_pte_from_task(taskp, addr);
                if (!ptep)continue;
                young = test_and_clear_young(ptep);
                pte_count += young;
            }
        }

        // print mem size, 1 pte = 4 Byte
        seq_printf(m, "%lld ", pte_count * 4);
        seq_printf(m, "\n");
    }

    return 0;
}

static int stat_open(struct inode* inode, struct file* file)
{
    return single_open(file, show_stat, NULL);
}

static ssize_t proc_write(struct file* fp, const char __user* ubuf, size_t len, loff_t* pos)
{
    int pid, i;
    struct pid* pidp;

    if (*pos > 0) return -EFAULT;

    pr_info("Writing the proc file\n");

    if (len > MAX_SIZE) len = MAX_SIZE;
    if (copy_from_user(input, ubuf, len)) return -EFAULT;

    pid_cnt = 0;
    for (i = 0; i < len; i++) {
        if (input[i] == ' ' || i == 0) {
            if (sscanf(input + i, "%d", &pid) <= 0)continue;
            pidp = find_get_pid(pid);
            if (!pidp)continue;
            taskp_list[pid_cnt++] = get_pid_task(pidp, PIDTYPE_PID);
            pr_info("proc watch: adding %d to pid list\n", pid);
        }
    }
    pr_info("to pid list.");

    *pos += len;
    return len;
}


static const struct proc_ops proc_ops = {
    .proc_open = stat_open,
    .proc_write = proc_write,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = seq_release,
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