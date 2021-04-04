# Dumb User Interface

***This Project is currently not working. It is now only a concept and in development***

The *Dumb User Interface* tries to be a minimalist *User Interface API*.

To build an own application it has to have the following structure.

```
[Script / Backend]        [Communicator]        [Root / Frontend]

[Startup Routine]  ->   [Write Communicator]  ->  [Root Window]

[After Interface Creation]

[Runtime Routine]  <-   [Read Communicator]   <-  [Root Window]

[Runtime Routine calls Feedback Processor]

[Feedback Processor] -> [Write Communicator]  ->  [Root Window]

```

In shell script it looks like this:

``` sh
# Start dui_root and save its pid in 'rpid'
dui_root & rpid=$!

# Wait until fifo session file is created

until [ -e "/tmp/dui/$rpid" ]
do
	printf "waiting for session FIFO\r"
done
echo

# Now do here your GUI build

# Wait for dui_root to exit
wait $rpid
```
