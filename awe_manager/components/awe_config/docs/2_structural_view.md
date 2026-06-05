# AWE CONFIG: Internal Architecture

There is no further sub-component included by awe_CONFIG component.

All API methods work on an internal dictionary.

The dictionary holds a list of configuration items, each item consists of three strings:

- key - defines which configuration item this is
- description - a description of the item, meaning/purpose
- value - the value of the item encoded as a string.

The values are stored as string values only. It is the task of the user of the awe_CONFIG API to cast or interpret this string data.
