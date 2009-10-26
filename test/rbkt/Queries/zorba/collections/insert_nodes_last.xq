import module namespace ddl = "http://www.zorba-xquery.com/module/ddl";
import module namespace dc = "http://www.zorba-xquery.com/module/dynamic-context";
import datamodule namespace ns = "http://example.org/datamodule/" at "collections.xqdata";

declare sequential function local:create() 
{
  ddl:create-collection(xs:QName("ns:test1"));
  ddl:insert-nodes-first(xs:QName("ns:test1"), for $i in 1 to 10 return <a> { $i } </a>);
  exit returning (fn:count(dc:collection(xs:QName("ns:test1"))) eq 10);
};

declare sequential function local:insert() 
{
  ddl:insert-nodes-last(xs:QName("ns:test1"), for $i in 11 to 13 return <b> { $i } </b>);
  exit returning dc:collection(xs:QName("ns:test1"));
};


declare variable $x := 0;

set $x := local:create();

if (fn:not($x)) then
  fn:false()
else 
  local:insert();
