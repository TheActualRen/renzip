## Relevant Commands
To run renzip, all you should need to do to generate the exe is:

```bash
make
```

To use renzip:
```
./renzip -e -2 input_file output_file
```

## Context
- `./renzip` is the exe name.
- `-e` is the flag for encoding.
- `-2` is the blocktype compression method (0 being the least amount of compression. 2 being the most)
- `input_file` this is the input file. For example, it could be something like `input.txt`
- `output_file` is the gunzip compressed file. You should ideally name it `.gz` at the end. For example `encoded.gz`

## Compatibility 
- If all goes well you can take your `encoded.gz` file and use the actual gunzip utility on it.
- Try:

```bash
gunzip encoded.gz
```
- This should decode the gunzip file created by gunzip to produce `encoded`
