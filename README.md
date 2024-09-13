Help order device tree source file node

# Build
    make

# Usage
  Order device tree nodes
  
    dt-format your_dts_file

  Compare two device-tree source file by omit node place change
  
    git difftool -y --extcmd=<path>/c.sh [git diff options]

# Order rule prosperity
- if there @, use order by address after @
- if there label: node_name, order by node_name
- property ahead child node
- property order
  -  compatible
  -  reg
  -  reg-names
  -  ranges
  -  #interrupt-cells
  -  interrupt-controller
  -  interrupts
  -  interrupt-names
  -  #gpio-cells
  -  gpio-controller
  -  gpio-ranges
  -  #address-cells
  -  #size-cells
  -  clocks
  -  clock-names
  -  assigned-clocks
  -  assigned-clock-parents
  -  assigned-clock-rates
  -  dmas
  -  dma-names
- status always last one
- alperbate order 
