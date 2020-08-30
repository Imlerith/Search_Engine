# Search_Engine
Implements a simple search engine in a database of text documents with support for parallel updating and querying in a thread-safe way.

The search server accepts an input stream of documents and constructs an *inverted index* mapping the words to the documents in which the word is encountered.

After that, queries can be run on the documents' database: the output for each query is at most five most relevant documents for this query. Relevance is measured by the sum of total times each word in the query is encountered in a document.

Queries and database updates can run in parallel, therefore thread safety is realized via mutex in an asynchronous processing setup.
