! longint.h - support for signed and unsigned 4 byte integers in Inform.
!
! Mainly by Chris Hall - c@pobox.co.uk.
! Signed integer support by Francis Irving - francis@pobox.co.uk.
! Please email us with any errors, omissions or if you've 
! found this code useful.
!
! For example use of this module, see longint.inf, which should be
! available from the same place you got this file.
!
! For a complete Inform program using this module see
! http://www.meta.demon.co.uk/zbefunge.html
!
! (If you need to edit this file, note that indentations are 4 spaces
! and tabs are not used.)
!
! This source code is distributed free, but remains
! Copyright 1997-1998 Chris Hall and Francis Irving.  Release 1.

System_file;

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

! Functions beginning "LongSign" work on signed integers.
! Functions beginning "LongUnsign" work on unsigned integers.
! Functions beginning "Long" are generally applicable.

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! Printing.

Array _LongNegTemp0->4;
Array _LongNegTemp1->4;

! Print a signed long.  Use by writing "print (longsign)x;".
[longsign n;
    LongSignOutputAsChars(n, LongPrintChar);
];

! Print an unsigned long.  Use by writing "print (longunsign)x;".
[longunsign n;
    LongUnsignOutputAsChars(n, LongPrintChar);
];

! Generalised printing.
! This takes "n" and outputs it a character at
! a time, calling function "fn".  Returns the length
! in characters.
[LongSignOutputAsChars n fn
        len;
    len = 0;
    if (LongSignIsNeg(n))
    {
        indirect(fn, '-');
        ++len;
    }
    LongSignAbsAssign(_LongNegTemp0, n);
    len = len + LongNum4(_LongNegTemp0->0,_LongNegTemp0->1,_LongNegTemp0->2,_LongNegTemp0->3, fn);
    return len;
];

! As signed version above.
[LongUnsignOutputAsChars n fn
        len;
    len = LongNum4(n->0,n->1,n->2,n->3, fn);
    return len;
];

! Internal function.
[LongPrintChar ch;
    print (char)ch;
];

! Internal function.
[LongNum4 a b c d fn
        l len;
    len = 0;
    b=b+256*(a%10); a=a/10;
    c=c+256*(b%10); b=b/10;
    d=d+256*(c%10); c=c/10;
    l=d%10;         d=d/10;
    if(a||b||c||d)
    {
        len = len + LongNum4(a,b,c,d, fn);
    }
    indirect(fn, '0'+l);
    ++len;

    return len;
];

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! Simple testing and assigning

! Test to see if a signed long is negative.
[LongSignIsNeg a;
    if ((a->0)&$80)
        rtrue;
    else
        rfalse;
];

! Test to see if long is zero.  Signed or unsigned.
[LongIsZero a;
    if ((a->0 == 0) && (a->1 == 0) && (a->2 == 0) && (a->3 == 0))
        rtrue;
    else
        rfalse;
];

! Sets values into a signed or unsigned long.
! The appropriate encoding is left up to the caller:
! 1. We store our longs in big-endian (most significant first) order.
! 2. a&$80 is non-zero for negative signed integers.
! You might want to call this with parameters (0, 0, 0, x) to 
! set a value from 0 to 255.  Then use LongSignNegAssign to
! make it negative.
[LongSet l a b c d;   ! l=((a*256+b)*256+c)*256+d
    l->0=a;
    l->1=b;
    l->2=c;
    l->3=d;
];

! Copy either a signed or unsigned long.
[LongAssign d s;      ! d=s
    d->0=s->0;
    d->1=s->1;
    d->2=s->2;
    d->3=s->3;
];

! d=-s  d and s can be the same variable
[LongSignNegAssign d s  
        t;
    LongAssign(d, s);
    LongNot(d);
    t=d->3+1;       d->3=t%256;
    t=d->2+t/256; d->2=t%256;
    t=d->1+t/256; d->1=t%256;
    t=d->0+t/256; d->0=t%256;
];

! d=positive part of s
[LongSignAbsAssign d s; 
    if (LongSignIsNeg(s))
        LongSignNegAssign(d, s);
    else
        LongAssign(d, s);
];

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! Addition and subtraction.

! r=a+b;   (r may be same variable as a or b)
! Works for signed or unsigned longs.
[LongAdd r a b        
         t;
    t=a->3+b->3;       r->3=t%256;
    t=a->2+b->2+t/256; r->2=t%256;
    t=a->1+b->1+t/256; r->1=t%256;
    t=a->0+b->0+t/256; r->0=t%256;
];

! r=a-b;   (r may be same variable as a or b)
! Works for signed or unsigned longs.
[LongSub r a b
        t;
    t=256+a->3-b->3;         r->3=t%256;
    t=256+a->2-b->2-(t<256); r->2=t%256;
    t=256+a->1-b->1-(t<256); r->1=t%256;
    t=256+a->0-b->0-(t<256); r->0=t%256;
];                            

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! Multiplication and division.

! r=a*b;   (r may _not_ be same variable as a or b)
! Works for signed or unsigned longs.  Note that the
! destination variable must be a different variable
! from both of the source variables.
[LongMul r a b        
        t;
    t=(a->3%16)*(b->3%16);      r->3=t%16;
    t=(a->3%16)*(b->3/16)
        +(a->3/16)*(b->3%16)+t/16; r->3=r->3+16*(t%16);
    t=(a->3%16)*(b->2%16)
        +(a->3/16)*(b->3/16)
        +(a->2%16)*(b->3%16)+t/16; r->2=t%16;
    t=(a->3%16)*(b->2/16)
        +(a->3/16)*(b->2%16)
        +(a->2%16)*(b->3/16)
        +(a->2/16)*(b->3%16)+t/16; r->2=r->2+16*(t%16);
    t=(a->3%16)*(b->1%16)
        +(a->3/16)*(b->2/16)
        +(a->2%16)*(b->2%16)
        +(a->2/16)*(b->3/16)
        +(a->1%16)*(b->3%16)+t/16; r->1=t%16;
    t=(a->3%16)*(b->1/16)
        +(a->3/16)*(b->1%16)
        +(a->2%16)*(b->2/16)
        +(a->2/16)*(b->2%16)
        +(a->1%16)*(b->3/16)
        +(a->1/16)*(b->3%16)+t/16; r->1=r->1+16*(t%16);
    t=(a->3%16)*(b->0%16)
        +(a->3/16)*(b->1/16)
        +(a->2%16)*(b->1%16)
        +(a->2/16)*(b->2/16)
        +(a->1%16)*(b->2%16)
        +(a->1/16)*(b->3/16)
        +(a->0%16)*(b->3%16)+t/16; r->0=t%16;
    t=(a->3%16)*(b->0/16)
        +(a->3/16)*(b->0%16)
        +(a->2%16)*(b->1/16)
        +(a->2/16)*(b->1%16)+t/16;
    t=(a->1%16)*(b->2/16)
        +(a->1/16)*(b->2%16)
        +(a->0%16)*(b->3/16)
        +(a->0/16)*(b->3%16)+t;    r->0=r->0+16*(t%16);
];

Array _LongDMTemp0->4;
Array _LongDMTemp1->4;

! d=a/b; m=a%b   (d and m may be same variable as a or b)
! Works for unsigned longs only.
[LongUnsignDivMod d m a b   
        t;

    ! Division by zero!  The user is encouraged to test for this
    ! himself before calling the various division functions.  We
    ! simply force the interpreter to perform a divide by zero.
    if(LongIsZero(b)){a=0;a=a/0;}

    LongAssign(_LongDMTemp1,b);
    LongAssign(m,a);
    d->0=d->1=d->2=d->3=
    _LongDMTemp0->0=_LongDMTemp0->1=_LongDMTemp0->2=0; _LongDMTemp0->3=1;

    while(LongUnsignLE(_LongDMTemp1,m)){
		t=LongShl(_LongDMTemp0);
        if (t) break;
        LongShl(_LongDMTemp1);
    }
    for(::){
        if(LongShr(_LongDMTemp0,t)) break;
        t=0;
        if(~~t) LongShr(_LongDMTemp1);
        if(LongUnsignLE(_LongDMTemp1,m)){
            LongSub(m,m,_LongDMTemp1);
            LongAdd(d,d,_LongDMTemp0);
        }
    }
];

Array _LongDMTemp2->4;

! r=a/b;   (r may be same variable as a or b)
! Works for signed longs only.
[LongSignDiv r a b
        n;
    n = LongNegIndex(a, b,_LongNegTemp0,_LongNegTemp1);
    LongUnsignDivMod(r,_LongDMTemp2,_LongNegTemp0,_LongNegTemp1);
    LongUseNegIndex(n, r);
];

! r=a%b;   (r may be same variable as a or b)
! Works for signed longs only.
[LongSignMod r a b
        n;
    n = LongNegIndex(a, b,_LongNegTemp0,_LongNegTemp1);
    LongUnsignDivMod(_LongDMTemp2,r,_LongNegTemp0,_LongNegTemp1);
    LongUseNegIndex(n, r);
];

! Internal function.
! c=abs(a), d= abs(b) if a*b is negative, return that.
[LongNegIndex a b c d;
    LongSignAbsAssign(c, a);
    LongSignAbsAssign(d, b);
    if (LongSignIsNeg(a) ~= LongSignIsNeg(b))
        rtrue;
    else
        rfalse;
];

! Internal function.
! if n true, let r=-r
[ LongUseNegIndex n r;  
    if (n)
        LongSignNegAssign(r, r);
];

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! Comparison.

! Returns:
!  a<b -1
!  a>b 1
!  a==b 0);
! Works for signed longs only.
[LongSignCompare a b;     
    if (LongSignIsNeg(a) && (~~LongSignIsNeg(b))) return -1;
    if ((~~LongSignIsNeg(a)) && LongSignIsNeg(b)) return 1;

    if (~~LongSignIsNeg(a))
    {
        LongSignAbsAssign(_LongNegTemp0, a);
        LongSignAbsAssign(_LongNegTemp1, b);
    }
    else  
    {
        ! swap round if they are both negative
        LongSignAbsAssign(_LongNegTemp0, b);
        LongSignAbsAssign(_LongNegTemp1, a);
    }

    return LongUnsignCompare(_LongNegTemp0, _LongNegTemp1);
];
 
! Returns:
!  a<b -1
!  a>b 1
!  a==b 0);
! Works for unsigned longs only.
[LongUnsignCompare a b;
    if(a->0<b->0) return(-1);
    if(a->0>b->0) rtrue;
    if(a->1<b->1) return(-1);
    if(a->1>b->1) rtrue;
    if(a->2<b->2) return(-1);
    if(a->2>b->2) rtrue;
    if(a->3<b->3) return(-1);
    return(a->3>b->3);
];

! Unsigned versions.
[LongUnsignLT a b; return(LongUnsignCompare(a,b)< 0);]; ! return(a< b);
[LongUnsignLE a b; return(LongUnsignCompare(a,b)<=0);]; ! return(a<=b);
[LongUnsignGT a b; return(LongUnsignCompare(a,b)> 0);]; ! return(a> b);
[LongUnsignGE a b; return(LongUnsignCompare(a,b)>=0);]; ! return(a>=b);
[LongUnsignEQ a b; return(LongUnsignCompare(a,b)==0);]; ! return(a==b);
[LongUnsignNE a b; return(LongUnsignCompare(a,b)~=0);]; ! return(a~=b);

! Signed versions.
[LongSignLT a b; return(LongSignCompare(a,b)< 0);]; ! return(a< b);
[LongSignLE a b; return(LongSignCompare(a,b)<=0);]; ! return(a<=b);
[LongSignGT a b; return(LongSignCompare(a,b)> 0);]; ! return(a> b);
[LongSignGE a b; return(LongSignCompare(a,b)>=0);]; ! return(a>=b);
[LongSignEQ a b; return(LongSignCompare(a,b)==0);]; ! return(a==b);
[LongSignNE a b; return(LongSignCompare(a,b)~=0);]; ! return(a~=b);

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! Bitwise comparisons.

! These bitwise operators aren't really signed or unsigned.

! a=(a<<1+t); return(bit lost off end);
[LongShl a t;
    t=a->3*2+t;     a->3=t%256;
    t=a->2*2+t/256; a->2=t%256;
    t=a->1*2+t/256; a->1=t%256;
    t=a->0*2+t/256; a->0=t%256;
    return(t/256);
];

! a=((a+(t<<32))>>1); return(bit lost off end);
[LongShr a t;
    t=a->0+256*t;     a->0=t/2;
    t=a->1+256*(t%2); a->1=t/2;
    t=a->2+256*(t%2); a->2=t/2;
    t=a->3+256*(t%2); a->3=t/2;
    return(t%2);
];

! a=~a, bitwise not
[LongNot a;
    a->0 = ~(a->0);
    a->1 = ~(a->1);
    a->2 = ~(a->2);
    a->3 = ~(a->3);
];
