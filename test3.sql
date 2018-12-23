USE orderDB_small;
DESC website;
select * from website;
select title,website.url from book,website where book.id=200013 and website.name='tmall';
select title,price from book,price where book.id=200014 and book.id =price.book_id;
select title,price,url from book,price,website where book.id=200014 and book.id =price.book_id and price.website_id=website.id;