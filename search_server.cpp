#include "search_server.h"

#include "parse.h"
#include "iterator_range.h"

#include <algorithm>
#include <future>
#include <numeric>

InvertedIndex::InvertedIndex(istream& document_input) {
	// builds the inverted index
	for (string current_document; getline(document_input, current_document);) {
		docs.push_back(move(current_document));
		size_t docid = docs.size() - 1;
		for (string_view word : SplitIntoWordsView(docs.back())) {
			auto& docids = index[word];
			if (!docids.empty() && docids.back().docid == docid) {
				++docids.back().hitcount;
			} else {
				docids.push_back( { docid, 1 });
			}
		}
	}
}

const vector<InvertedIndex::Entry>& InvertedIndex::Lookup(
		string_view word) const {
	// does search in the inverted index
	static const vector<Entry> empty;
	if (auto it = index.find(word); it != index.end()) {
		return it->second;
	} else {
		return empty;
	}
}

void UpdateIndex(istream& document_input, Synchronized<InvertedIndex>& index) {
	// updates the index
	InvertedIndex new_index(document_input);
	swap(index.GetAccess().ref_to_value, new_index);
}

void ProcessSearches(istream& query_input, ostream& search_results_output,
		Synchronized<InvertedIndex>& index_handle) {

	/*
	 * This does the actual search, fills in the vector of relevance
	 * metrics for documents and puts them into correspondence to
	 * document ids
	 * */

	vector<size_t> docid_count;
	vector<int64_t> docids;

	for (string current_query; getline(query_input, current_query);) {
		const auto words = SplitIntoWordsView(current_query);

		{
			// thread-safe access to the inverted index
			auto access = index_handle.GetAccess();

			/* resize the vectors of ids and counts because the size of the
			 * vectors may get changed by parallel index updates
			 * (processing of queries can be done in parallel with
			 * adding new documents into the database)
			 * */
			const size_t doc_count = access.ref_to_value.GetDocuments().size();
			docid_count.assign(doc_count, 0);
			docids.resize(doc_count);

			auto& index = access.ref_to_value;
			for (const auto& word : words) {
				for (const auto& pair_ : index.Lookup(word)) {
					docid_count[pair_.docid] += pair_.hitcount;
				}
			}
		}

		// fill in the documents' indices
		iota(docids.begin(), docids.end(), 0);

		{
			// get top 5 most relevant documents
			partial_sort(begin(docids), Head(docids, 5).end(), end(docids),
					[&docid_count](int64_t lhs, int64_t rhs) {
						return pair(docid_count[lhs], -lhs) > pair(docid_count[rhs], -rhs);
					});
		}

		// write the top documents into the output stream
		search_results_output << current_query << ':';
		for (size_t docid : Head(docids, 5)) {
			const size_t hit_count = docid_count[docid];
			if (hit_count == 0) {
				break;
			}

			search_results_output << " {" << "docid: " << docid << ", "
					<< "hitcount: " << hit_count << '}';
		}
		search_results_output << '\n';
	}
}

void SearchServer::UpdateDocumentBase(istream& document_input) {
	// asynchronous updating of the database
	async_tasks.push_back(async(UpdateIndex, ref(document_input), ref(index)));
}

void SearchServer::AddQueriesStream(istream& query_input,
		ostream& search_results_output) {
	// asynchronous execution of the queries
	async_tasks.push_back(
			async(ProcessSearches, ref(query_input), ref(search_results_output),
					ref(index)));
}
