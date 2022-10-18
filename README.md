use -X mongodb_api -y mongodb_key for logging to a mongodb cluster,
cluster must be named alpinevms , database must be vlmcsd and collection mustbe log
ex: vlmcsd -X https://data.mongodb-api.com/app/data-562H/endpoint/data/v1/action/insertOne -Y xxxxMmzLYyT6qM9AJhdzkjzfzic3F0zOKnfzdeojzfdnizefz  

To view the documentation cd to the directory containing the distribution
files and type

man man/vlmcsd.8
	to see documentation for vlmcsd

man man/vlmcs.1
	to see documentation for vlmcs

man man/vlmcsd.7
	to see general documentation for kms

If you don't have man, you may also use the .txt, .html and .pdf files
in the man directory

For renewing the kms database I used License Manager (from https://forums.mydigitallife.net/threads/miscellaneous-kms-related-developments.52594/page-44)
With that I exported the database as vlmcsd and copy the variable in  kmsdata.c and kmsdata-full.c
