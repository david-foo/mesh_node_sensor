#include <minode.h>

#include "sensor_srv.h"
#include "sensors.h"

static void temp_on_new_sampling(struct minode_dht_device *dev);
static int sensor_dht_init(struct sensor *s);

MESH_CHARACTER_FIELD_DEFINE(temperature_8, temperature_8, sint8);

MESH_CHARACTER_DEFINE_EX(
	temperature_8,
	{
		&MESH_CHARACTER_FIELD(temperature_8)
	}
);

MESH_DEVICE_PROPERTY_DEFINE(present_ambient_temperature, temperature_8);

MINODE_DHT_DEVICE_DEFINE(temp_sensor_dev, A0, &dht_sensor, temp_on_new_sampling);

static struct sensor_prop sensor_dht_prop = {
	.prop = &present_ambient_temperature,
};

static struct sensor_prop *channels[] = {
	&sensor_dht_prop
};
struct sensor dht_sensor = {
	.name = "DHT11",
	.channel = channels,
	.channels_count = ARRAY_SIZE(channels),
	.dev = &temp_sensor_dev,
	.init = sensor_dht_init,
	.deinit = NULL
};

static void temp_on_new_sampling(struct minode_dht_device *dev)
{
	struct sensor *sens;
	struct mesh_characteristic_field *field;
  struct sensor_value temp_data;

	sens = dev->user_data;
	field = sens->channel[0]->prop->character->field[0];

  minode_dht_ambient_temp_get(dev, &temp_data);
	field->value[0] = temp_data.val1;

  printk("DHT11[temperature] attached on %s new sampling - ambient_temp: %d\n",
      dev->connector, temp_data.val1);
}

static int sensor_dht_init(struct sensor *s)
{
	struct minode_dht_device *dev;
	int err;

	if (!s) {
		return -EINVAL;
	}

	dev = s->dev;
	if (!dev) {
		return -ENODEV;
	}

	err = minode_dht_init(dev);
	if (err < 0) {
		return err;
	}

	minode_dht_start_sampling(&temp_sensor_dev, 1000 * 60);

	sensor_dht_prop.desc.prop_id = sensor_dht_prop.prop->id->uuid;
	sensor_dht_prop.desc.positive_tolerance = 0;
	sensor_dht_prop.desc.negative_tolerance = 0;
	sensor_dht_prop.desc.measurement_period = 88;		// 10s
	sensor_dht_prop.desc.update_interval = 107;			// 60s
	sensor_dht_prop.desc.sampling_function = 1;			// Instantaneous

	return 0;
}