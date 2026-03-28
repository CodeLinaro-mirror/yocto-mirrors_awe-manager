# AWE CONFIG: Resource View

The awe_CONFIG component in a future release may support reading or storing configuration from or to a file. There are no load/save APIs defined yet.

The file format of a configuration file would be very simple. It just consists of lines of text strings, with a specific delimiter.

When reading the file into memory, a simple strtok() is used to parse and to substitute the delimiter with a '\0' character.

```
key1,value1,description
key2,value2,description
```

Other than this character substitution there is no parsing during load time.

Using strings for the keys allows for very easy structuring of configuration data, like: mgr.api.log.level or mgr.awc.log.level.
