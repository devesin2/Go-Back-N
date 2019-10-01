simple Tic
{
    parameters:
        @display("i=block/routing");
        //volatile int data_rate @unit(Mbps);
    gates:
        input in;
        output out;
}

simple Toc
{
    parameters:
        @display("i=block/routing");
        int win_size;
    gates:
        input in;
        output out;
}

network TictocX
{
    @display("bgb=298,347");
    submodules:
        tic: Tic {
            @display("i=,cyan;p=75,72");
        }
        toc: Toc {
            @display("i=,gold;p=208,185");
        }
    connections:
        tic.out -->{ delay = 100ms; datarate = 100Mbps; }--> toc.in;
        tic.in <-- { delay = 100ms; datarate = 100Mbps; }<-- toc.out;
}
