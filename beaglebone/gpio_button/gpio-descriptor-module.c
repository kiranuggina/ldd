#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>		/* For Platform devices */
#include <linux/gpio/consumer.h>		/* For GPIO Descriptor */
#include <linux/interrupt.h>			/* For IRQ */
#include <linux/of.h>				/* For DT */

/*
Let us consider the below mapping in device tree:
foo_device {
	compatible = "omap_gpio, gpio-descriptor-sample";
	led-gpios = <&gpio2 2 GPIO_ACTIVE_HIGH>; 	//led 		P8_7	GPIO_66 	
	button-gpios = <&gpio2 3 GPIO_ACTIVE_LOW>;	//button 	P8_8	GPIO_67
};
*/

static struct gpio_desc *led, *button;
static unsigned int irq;
static bool ledOn = 0;

static irq_handler_t btn1_pressed_irq_handler(unsigned int irq, void * dev_id, struct pt_regs *regs)
{
	ledOn = !ledOn;
	/* read the button value and change the led state */
	gpiod_set_value(led,ledOn);
	pr_info("led state: %d\n",gpiod_get_value(led));	
	pr_info("btn1 interrrupt: Interrupt! btn1 state is %d\n",gpiod_get_value(button));
	return (irq_handler_t)IRQ_HANDLED;
}

static const struct of_device_id gpiod_dt_ids[] = {
	{ .compatible = "omap_gpio, gpio-descriptor-sample",},
	{}
};

static int my_pdrv_probe(struct platform_device *pdev)
{
	int retval;
	struct device *dev = &pdev->dev;

	/* Use gpiod_get/gpiod_get_index() along with the flags in order to configure 
	the GPIO direction and an initial value in a single function vall. */
	led = gpiod_get(dev, "led", GPIOD_OUT_LOW);
	gpiod_export(led,false);

	/* configure the gpio buttons as input */
	button = gpiod_get(dev,"button",GPIOD_IN);
	gpiod_set_debounce(button,200);
	gpiod_export(button,false);

	irq = gpiod_to_irq(button);
	retval = request_threaded_irq(irq,NULL,
				(irq_handler_t)btn1_pressed_irq_handler,
				IRQF_TRIGGER_RISING | IRQF_ONESHOT,
				"gpio-descriptor-sample",NULL);
	pr_info("gpio descriptor example driver probed!\n");
	return 0;
	
}

static int my_pdrv_remove(struct platform_device *pdev)
{
	free_irq(irq,NULL);
	gpiod_set_value(led,0);
	gpiod_put(led);
	gpiod_put(button);
	pr_info("good bye, gpio descriptor example driver unloaded!\n");
	return 0;
}

static struct platform_driver mypdrv = {
	.probe = my_pdrv_probe,
	.remove = my_pdrv_remove,
	.driver = {
		.name = "gpio_descriptor_sample",
		.of_match_table = of_match_ptr(gpiod_dt_ids),
		.owner = THIS_MODULE,
	},
};

module_platform_driver(mypdrv);

MODULE_AUTHOR("Kiran Kumar U <suryakiran104@gmail.com>");
MODULE_LICENSE("GPL");

