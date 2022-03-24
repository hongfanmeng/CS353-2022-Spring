#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define MAX_SIZE 128
#define ID "520030910125"

static int operand1;
module_param(operand1, int, 0);
static char* operator;
module_param(operator, charp, 0);
static int operand2[MAX_SIZE];
static int ninp;
module_param_array(operand2, int, &ninp, 0);

static struct proc_dir_entry* proc_ent;
static struct proc_dir_entry* proc_dir;
static char input[MAX_SIZE];
static char output[MAX_SIZE];
int out_len, in_len;

static ssize_t proc_read(struct file* fp, char __user* ubuf, size_t len, loff_t* pos) {
    int i;
    ssize_t ret;
    out_len = 0;
    for (i = 0; i < ninp; i++) {
        int tmp;
        if (strcmp(operator, "add") == 0) {
            tmp = operand1 + operand2[i];
        }
        else {
            tmp = operand1 * operand2[i];
        }
        out_len += sprintf(output + out_len, "%d", tmp);
        if (i != ninp - 1) {
            output[out_len++] = ',';
        }
    }
    output[out_len++] = '\n';

    ret = out_len;
    if (*pos >= out_len || copy_to_user(ubuf, output, out_len)) {
        pr_info("copy_to_user failed\n");
        ret = 0;
    }
    else {
        pr_info("procfile read %s\n", fp->f_path.dentry->d_name.name);
        *pos += len;
    }
    return ret;
}

static ssize_t proc_write(struct file* fp, const char __user* ubuf, size_t len, loff_t* pos) {
    int tmp;

    in_len = len;
    if (in_len > MAX_SIZE)
        in_len = MAX_SIZE;

    if (copy_from_user(input, ubuf, in_len))
        return -EFAULT;

    input[in_len & (MAX_SIZE - 1)] = '\0';
    pr_info("procfile write %s\n", input);

    sscanf(input, "%d", &tmp);
    operand1 = tmp;
    pr_info("update operand1: %d\n", operand1);

    return in_len;
}

static const struct proc_ops proc_ops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

static int __init proc_init(void) {
    proc_dir = proc_mkdir(ID, NULL);
    if (NULL == proc_dir) {
        proc_remove(proc_dir);
        pr_alert("Error: Could not initialize /proc/%s\n", ID);
        return -ENOMEM;
    }

    proc_ent = proc_create("calc", 0644, proc_dir, &proc_ops);
    if (NULL == proc_ent) {
        proc_remove(proc_ent);
        proc_remove(proc_dir);
        pr_alert("Error: Could not initialize /proc/%s/%s\n", ID, "calc");
        return -ENOMEM;
    }

    pr_info("/proc/%s/%s created\n", ID, "calc");
    return 0;
}

static void __exit proc_exit(void) {
    proc_remove(proc_ent);
    pr_info("/proc/%s/%s removed\n", ID, "calc");
    proc_remove(proc_dir);
    pr_info("/proc/%s removed\n", ID);
}

module_init(proc_init);
module_exit(proc_exit);
MODULE_LICENSE("GPL");