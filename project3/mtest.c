#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/highmem.h>

#define MAX_SIZE 128

static struct proc_dir_entry* proc_ent;
static char input[MAX_SIZE];
static char output[MAX_SIZE];
static int out_len;

static struct page* get_page_from_task(struct task_struct* task, int addr);

enum operation {
    OP_READ, OP_WRITE
};

static ssize_t proc_read(struct file* fp, char __user* ubuf, size_t len, loff_t* pos)
{
    int count; /* the number of characters to be copied */
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
    // TODO: parse the input, read/write process' memory

    static char operator;
    int pid, content;
    unsigned long addr;
    struct task_struct* task;
    struct page* pg;
    void* p;

    size_t in_len = len < MAX_SIZE ? len : MAX_SIZE;

    if (copy_from_user(input, ubuf, in_len))
        return -EFAULT;

    input[in_len & (MAX_SIZE - 1)] = '\0';
    pr_info("procfile write %s\n", input);

    if (input[0] == 'r') {
        sscanf(input, "%c %d %lx", &operator, &pid, &addr);
        pr_info("cmd: %c, pid: %d, addr: %lx\n", operator, pid, addr);
    }
    else {
        sscanf(input, "%c %d %lx %d", &operator, &pid, &addr, &content);
        pr_info("cmd: %c, pid: %d, addr: %lx, content: %d\n", operator, pid, addr, content);
    }

    task = get_pid_task(find_vpid(pid), PIDTYPE_PID);
    pr_info("get task of pid: %d\n", task->pid);
    pg = get_page_from_task(task, addr);

    if (pg == NULL) {
        pr_info("fail to get page");
        return in_len;
    }

    p = kmap_local_page(pg);

    pr_info("get page of addr: %p\n", p);

    pr_info("content: %d", *(char*)(p));

    kunmap_local(p);

    return in_len;
}


static struct page* get_page_from_task(struct task_struct* task, int addr) {
    pgd_t* pgd;
    p4d_t* p4d;
    pud_t* pud;
    pte_t* pte;
    pmd_t* pmd;
    unsigned long pfn;

    pgd = pgd_offset(task->mm, addr); /* get the pgd entry */
    if (pgd_none(*pgd) || pgd_bad(*pgd)) {
        pr_info("fail to get pgd");
        return NULL;
    }

    // p4d = p4d_offset(pgd, addr); /* get the p4d entry */
    // if (p4d_none(*p4d) || p4d_bad(*p4d)) {
    //     pr_info("fail to get p4d");
    //     return NULL;
    // }

    pud = pud_offset(pgd, addr); /* get the pud entry */
    if (pud_none(*pud) || pud_bad(*pud)) {
        pr_info("fail to get pud");
        return NULL;
    }

    pmd = pmd_offset(pud, addr);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) {
        pr_info("fail to get pmd");
        return NULL;
    }

    pte = pte_offset_kernel(pmd, addr); /* get the pte entry */
    if (pte_none(*pte)) {
        pr_info("fail to get pte");
        return NULL;
    }

    pfn = pte_pfn(*pte);

    // pfn = pgd_pfn(*pgd);

    return pfn_to_page(pfn);
}

static const struct proc_ops proc_ops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

static int __init mtest_init(void)
{
    proc_ent = proc_create("mtest", 0666, NULL, &proc_ops);
    if (!proc_ent)
    {
        proc_remove(proc_ent);
        pr_alert("Error: Could not initialize /proc/mtest\n");
        return -EFAULT;
    }
    pr_info("/proc/mtest created\n");
    return 0;
}

static void __exit mtest_exit(void)
{
    proc_remove(proc_ent);
    pr_info("/proc/mtest removed\n");
}

module_init(mtest_init);
module_exit(mtest_exit);
MODULE_LICENSE("GPL");