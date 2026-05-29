#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/fs.h>
#include <linux/sysfs.h>
#include <linux/acpi.h>


struct cix_gpio_drvdata {
    struct gpio_desc *gpio0;
    struct gpio_desc *gpio1;
    struct gpio_desc *gpio2;
};

static ssize_t pin0_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct cix_gpio_drvdata *data = dev_get_drvdata(dev);
    int val = gpiod_get_value(data->gpio0);
    return sprintf(buf, "%d\n", val);
}

static ssize_t pin1_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct cix_gpio_drvdata *data = dev_get_drvdata(dev);
    int val = gpiod_get_value(data->gpio1);
    return sprintf(buf, "%d\n", val);
}

static ssize_t pin2_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct cix_gpio_drvdata *data = dev_get_drvdata(dev);
    int val = gpiod_get_value(data->gpio2);
    return sprintf(buf, "%d\n", val);
}

static DEVICE_ATTR_RO(pin0); 
static DEVICE_ATTR_RO(pin1); 
static DEVICE_ATTR_RO(pin2); 

static struct attribute *cix_gpio_attrs[] = {
    &dev_attr_pin0.attr,
    &dev_attr_pin1.attr,
    &dev_attr_pin2.attr,
    NULL,
};

static const struct attribute_group cix_gpio_group = {
    .attrs = cix_gpio_attrs,
};

static int cix_gpio_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct cix_gpio_drvdata *data;
    int ret;

    dev_info(dev, "CIX GPIO Reader: Found device, probing...\n");

    data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
    if (!data)
        return -ENOMEM;

    data->gpio0 = devm_gpiod_get(dev, "ID0", GPIOD_IN);
    if (IS_ERR(data->gpio0)) return PTR_ERR(data->gpio0);

    data->gpio1 = devm_gpiod_get(dev, "ID1", GPIOD_IN);
    if (IS_ERR(data->gpio1)) return PTR_ERR(data->gpio1);

    data->gpio2 = devm_gpiod_get(dev, "ID2", GPIOD_IN);
    if (IS_ERR(data->gpio2)) return PTR_ERR(data->gpio2);

    platform_set_drvdata(pdev, data);

    ret = sysfs_create_group(&dev->kobj, &cix_gpio_group);
    if (ret) {
        dev_err(dev, "Failed to create sysfs files\n");
        return ret;
    }

    dev_info(dev, "CIX GPIO Reader: Probe successfully. Check /sys/devices/platform/%s/\n", pdev->name);
    return 0;
}

static int cix_gpio_remove(struct platform_device *pdev)
{
    sysfs_remove_group(&pdev->dev.kobj, &cix_gpio_group);
    dev_info(&pdev->dev, "CIX GPIO Reader: Removed\n");
    return 0;
}

static const struct of_device_id cix_gpio_match[] = {
    { .compatible = "cix,gpio-reader", },
    { },
};
MODULE_DEVICE_TABLE(of, cix_gpio_match);

static struct platform_driver cix_gpio_driver = {
    .probe  = cix_gpio_probe,
    .remove = cix_gpio_remove,
    .driver = {
        .name = "cix_gpio_reader",
        .of_match_table = cix_gpio_match,
    },
};

module_platform_driver(cix_gpio_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huangpj");
MODULE_DESCRIPTION("Simple ACPI GPIO");
