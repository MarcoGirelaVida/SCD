/*
Crear con cobegin-coend y fork-join los grafos que se ven en la foto
*/

// Grafo a):
/*
Con fork-join
begin;
    PO
    fork P2,
    P1;
    P3;
    fork P5;
    P4;
    join P5;
    join P2;
    P6
end;

Con cobegin-coend
P0
cobegin
    P2
    begin
        P1
        P3
        cobegin
            P4, P5
        coend
    end
coend
P6
*/

/*
Grafo c)
P0;
fork P2
P1
P3
fork P4
join P2
P5
join P4
P6

No se puede hacer con cobegin coend, porque no terminan a la vez

*/