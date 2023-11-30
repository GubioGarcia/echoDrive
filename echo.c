#include <linux/module.h> // Necessário para todos os módulos
#include <linux/kernel.h> // Necessário para KERN_INFO
#include <linux/fs.h> // Necessário para as funções de dispositivo de caractere
#include <linux/cdev.h> // Necessário para cdev
#include <linux/uaccess.h> // Necessário para copy_to_user e copy_from_user

#define DEVICE_NAME "echodev" // Nome do dispositivo como aparecerá em /dev/echodev

static int major; // Major number atribuído ao nosso dispositivo
static char msg_buffer[256]; // Buffer para armazenar a string do usuário
static short msg_size; // Tamanho da string armazenada no buffer

// Chamado quando o dispositivo é aberto do espaço do usuário
static int dev_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "EchoDev: Dispositivo aberto\n");
    return 0;
}

// Chamado quando o dispositivo é lido do espaço do usuário
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int error_count = 0;

    // Copia a string do kernel para o espaço do usuário
    error_count = copy_to_user(buffer, msg_buffer, msg_size);
    
    if (error_count==0) { // Se for bem-sucedido, retorna o tamanho da mensagem
        printk(KERN_INFO "EchoDev: Enviou %d caracteres para o usuário\n", msg_size);
        return (msg_size=0); // Limpa o buffer e retorna 0
    } else {
        printk(KERN_INFO "EchoDev: Falhou em enviar %d caracteres para o usuário\n", error_count);
        return -EFAULT; // Retorna um erro de falha
    }
}

// Chamado quando o dispositivo é escrito do espaço do usuário
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    sprintf(msg_buffer, "%s(%zu letters)", buffer, len); // Formata a mensagem com o tamanho
    msg_size = strlen(msg_buffer); // Armazena o tamanho da mensagem
    printk(KERN_INFO "EchoDev: Recebeu %zu caracteres do usuário\n", len);
    return len;
}

// Chamado quando o dispositivo é fechado/liberado pelo espaço do usuário
static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "EchoDev: Dispositivo fechado\n");
    return 0;
}

// Estrutura que declara as funções de callback do dispositivo
static struct file_operations fops =
{
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

// Função chamada quando o módulo é carregado
int init_module(void) {
    printk(KERN_INFO "EchoDev: Registrando dispositivo de caractere\n");

    // Tenta registrar o dispositivo de caractere dinamicamente
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "EchoDev falhou ao registrar um major number\n");
        return major;
    }
    printk(KERN_INFO "EchoDev: registrado corretamente com major number %d\n", major);
    return 0;
}

// Função chamada quando o módulo é descarregado
void cleanup_module(void) {
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "EchoDev: Removendo módulo\n");
}

MODULE_LICENSE("GPL"); // Licença do módulo
MODULE_AUTHOR("Anon"); // Autor do Módulo
MODULE_DESCRIPTION("Um simples driver de dispositivo de caractere de Linux para echo"); // Descrição do módulo
MODULE_VERSION("0.1"); // Versão do módulo