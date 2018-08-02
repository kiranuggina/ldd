#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/gpio.h>
#include<linux/interrupt.h>

MODULE_AUTHOR("U Kiran Kumar");
MODULE_LICENSE("GPL");

static unsigned int led = 66;		//GPIO_66	P8_7
static unsigned int button = 67;	//GPIO_67	P8_8
static unsigned int irq;
static bool ledOn = 0;

static irq_handler_t button_pressed_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
	ledOn = !ledOn;
	gpio_set_value(led,ledOn);
	pr_info("GPIO Button interrupt! button state is %d\n",gpio_get_value(button));
	return (irq_handler_t)IRQ_HANDLED;
}

static int __init mygpio_init(void)
{
	int ret;
	if(!gpio_is_valid(led))
	{
		pr_info("Invalid led gpio");
		return -ENODEV;
	}
	if(!gpio_is_valid(button))
	{
		pr_info("Invalid button gpio");
		return -ENODEV;
	}
	
	gpio_request(led,"sysfs");
	gpio_direction_output(led,ledOn);
	gpio_export(led,false);

	gpio_request(button,"sysfs");
	gpio_direction_input(button);
	gpio_set_debounce(button,200);
	gpio_export(button,false);

	irq = gpio_to_irq(button);
/*	ret = request_irq(irq,
				(irq_handler_t)button_pressed_irq_handler,
				IRQF_TRIGGER_RISING,
				"mygpio-button-example",NULL);*/
	ret = request_threaded_irq(irq,NULL,
			(irq_handler_t)button_pressed_irq_handler,
			IRQF_TRIGGER_RISING | IRQF_ONESHOT,
			"mygpio-button-example",NULL);

	pr_info("mygpio button driver loaded\n");
	return 0;
}

static void __exit mygpio_exit(void)
{
	gpio_set_value(led,0);
	gpio_unexport(led);
	gpio_free(led);
	free_irq(irq,NULL);
	gpio_unexport(button);
	gpio_free(button);
	pr_info("mygpio button driver unloaded\n");
}

module_init(mygpio_init);
module_exit(mygpio_exit);

