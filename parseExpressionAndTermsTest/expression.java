a + b;
a - b;
a * b;
a / b;
a % b;
a & b;
a | b;
a ^ b;
a << b;
a >> b;
a < b;
a > b;
a <= b;
a >= b;
a == b;
a != b;
a && b;
a || b;
!a;
~a;
a++;
a--;
++a;
--a;
a ? b : c;
(a + b);
a = b;
a += b;
a -= b;
a *= b;
a /= b;
(int)a;
(double)b;
new A();
new B(1, 2, 3);
new C[10];
new int[5][6];
new String[]{"a", "b", "c"};
a.b;
a.b.c;
a[0];
a[0][1];
a.b[2];
a.b.c[3][4];
a.b().c();
a.b().c().d;
a.b(1, 2);
a.b().c(3, 4);
this.a;
this.a.b;
this.a().b;
null;
true;
false;
"Hello";
"Hello " + "World";
"a" + b;
a + "b";
a + b + "c";
'a';
'a' + b;
a + 'b';
1;
1 + 2;
1 - 2;
1 * 2;
1 / 2;
1 % 2;
1 << 2;
1 >> 2;
1 & 2;
1 | 2;
1 ^ 2;
1 && true;
true || false;
1 > 2;
1 < 2;
1 >= 2;
1 <= 2;
1 == 2;
1 != 2;
(1 + 2) * (3 - 4);
a = b ? c : d;
a[1] = b;
a[1][2] = b[3];
a.b[2] = c[3];
a = b + c[2];
a = (b + c) * d;
a.b().c().d = e;
this.a = b;
a = this.b;
this.a.b = c;
new A().b = c;
(new A().b).c = d;
a[0] = b[1];
a.b().c[0] = d[1];
a.b(c.d(e.f()));
f(a ? b : c);
g(a[0]);
h(a.b[0]);
a.b(c, d);
a.b().c(d);
a.b().c(d, e);
a = a.b(c ? d : e);
a = b[c.d(e)];
a = b ? c : d ? e : f;
a = (b ? c : d) ? e : f;
a = b ? c : (d ? e : f);
a = b ? (c ? d : e) : f;
new A(new B());
new A(new B().c);
new A(new B(c));
new A(c.d());
new A(b ? c : d);
a = b ? new C(d) : new E(f);
a = new A().b.c;
a = new A().b(c);
a = new A().b[c];
a = new A(b[c]);
a = (new A()).b(c);
a = (new A()).b[c];
a = (new A(b)).c;
a.b = new C(d);
a.b[c] = new D(e);
a = new A().b().c(d);
a = new A().b[c.d];
new A(b[c]).d = e;
new A(b[c]).d[e] = f;
a = b[c ? d : e];
a = b ? c[d] : e[f];
a = b ? c[d] : e[f];
a = b[c.d(e[f])];
new A(b[c.d(e)]);
a = b[c ? d : (e ? f : g)];
a = b ? c[d.e(f)] : g;
a = b[c[d.e(f)]];
a = b ? c[d.e(f)] : (g[h.i(j)]);
a = b[c[d.e(f) ? g[h.i(j)] : k]];
