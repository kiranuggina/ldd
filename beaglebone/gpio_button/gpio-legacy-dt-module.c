#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>      /* For platform devices */
#include <linux/interrupt.h>            /* For IRQ */
#include <linux/gpio.h>                 /* For Legacy integer based GPIO */
#include <linux/of_gpio.h>              /* For of_gpio* functions */
#include <linux/of.h>                   /* For DT*/


/*
 * Let us consider the node bellow
 *
 *    foo_device {
 *       compatible = "omap_gpio,gpio-legacy-sample";
 *       led-gpios = <&gpio2 2 GPIO_ACTIVE_HIGH>, 	// led 		P8_7	GPIO_66
 *       button-gpios = <&gpio2 3 GPIO_ACTIVE_HIGH>, 	// button	P8_8	GPIO_67
 *   };
 */

static unsigned int led, button;
static unsigned int irq;
static bool ledOn = 0;

static irq_handler_t button_pushed_irq_handler(int irq, void *dev_id)
{
	ledOn = !ledOn;
    	gpio_set_value(led, ledOn);
    	pr_info("gpio_button interrupt: Interrupt! gpio_button state is %d)\n", gpio_get_value(button));
    	return (irq_handler_t)IRQ_HANDLED;
}

static const struct of_device_id gpio_dt_ids[] = {
	{ .compatible = "omap_gpio, gpio-legacy-sample",},
	{}
};

static int my_pdrv_probe (struct platform_device *pdev)
{
    	int retval;
    	struct device_node *np = pdev->dev.of_node;

    	if (!np)
    		return -ENOENT;

    	/* fetch the gpio from dt */
	led = of_get_named_gpio(np, "led", 0);
    	button = of_get_named_gpio(np, "button", 0);

    	/* request the gpios */
	gpio_request(led, "sysfs");
    	gpio_request(button, "sysfs");
	
	/* set the direction input for button gpio */
    	gpio_direction_input(button);
	gpio_set_debounce(button,200);
	/* export the gpio to sysfs, flag false indicates dont change the state of the pin */
	gpio_export(button,false);
    
	/* set the direction output for led gpio */
    	gpio_direction_output(led, ledOn);	//Initially LedOff
	gpio_export(button,false);

    	irq = gpio_to_irq(button);
    	retval = request_threaded_irq(irq, NULL,
    	                        (irq_handler_t)button_pushed_irq_handler,
                            	IRQF_TRIGGER_RISING | IRQF_ONESHOT,
                            	"gpio-legacy-sample", NULL);

	pr_info("my gpio button platform driver! loaded\n");
    	return 0;
}

static int my_pdrv_remove(struct platform_device *pdev)
{
    	free_irq(irq, NULL);
	gpio_unexport(led);
    	gpio_free(led);
	gpio_unexport(button);
    	gpio_free(button);

    	pr_info("my platform gpio button driver! unloaded\n");
    	return 0;
}


static struct platform_driver mypdrv = {
    	.probe      = my_pdrv_probe,
    	.remove     = my_pdrv_remove,
    	.driver     = {
        	.name     = "gpio_legacy_sample",
        	.of_match_table = of_match_ptr(gpio_dt_ids),  
        	.owner    = THIS_MODULE,
    	},
};
module_platform_driver(mypdrv);

MODULE_AUTHOR("Kiran Kumar U <suryakiran104@gmail.com>");
MODULE_LICENSE("GPL");
