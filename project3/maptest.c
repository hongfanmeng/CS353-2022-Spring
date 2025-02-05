#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/highmem.h>

static struct proc_dir_entry* proc_ent;
static struct page* page;
static char content[] =
"Listen to me say thanks\n"
"Thanks to you, I'm warm all the time\n"
"I thank you\n"
"For being there\n"
"The world is sweeter\n"
"I want to say thanks\n"
"Thanks to you, love is in my heart\n"
"I thank you, for being there\n"
"To bring happiness\n";

static int proc_mmap(struct file* fp, struct vm_area_struct* vma)
{
    unsigned long size = vma->vm_end - vma->vm_start;
    return remap_pfn_range(vma, vma->vm_start, page_to_pfn(page), size, vma->vm_page_prot);
}

static const struct proc_ops proc_ops = {
    .proc_mmap = proc_mmap,
};

static int __init maptest_init(void)
{
    void* base;

    proc_ent = proc_create("maptest", 0666, NULL, &proc_ops);
    if (!proc_ent)
    {
        proc_remove(proc_ent);
        pr_alert("Error: Could not initialize /proc/maptest\n");
        return -EFAULT;
    }
    pr_info("/proc/maptest created\n");

    page = alloc_page(GFP_HIGHUSER_MOVABLE);

    base = kmap_local_page(page);

    memcpy(base, content, min(strlen(content), PAGE_SIZE));

    return 0;
}

static void __exit maptest_exit(void)
{
    proc_remove(proc_ent);
    pr_info("/proc/maptest removed\n");
    __free_page(page);
    pr_info("memory freed\n");
}

module_init(maptest_init);
module_exit(maptest_exit);
MODULE_LICENSE("GPL");