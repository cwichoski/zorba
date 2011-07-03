import module namespace ddl = "http://www.zorba-xquery.com/modules/store/static/collections/ddl";

import module namespace ns = "http://example.org/datamodule/" at "collections.xqdata";

declare namespace ann = "http://www.zorba-xquery.com/annotations";

declare %ann:sequential function local:test() {
  (ddl:create-collection($ns:http),ddl:create-collection($ns:http));
};

local:test()
