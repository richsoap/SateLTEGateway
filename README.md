# SateLTEGateway
Try to transfer satellite network signal(custom) to LTE signal(S1AP).
## use asn1c
```
asn1c *.asn -D <output_dir> -pd=all -no-gen-OER -no-gen-example
```
## notice!
call this before compiling
```
rm *oer*
```
