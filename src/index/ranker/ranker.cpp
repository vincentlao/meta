/**
 * @file ranker.cpp
 * @author Sean Massung
 */

#include "corpus/document.h"
#include "index/inverted_index.h"
#include "index/postings_data.h"
#include "index/ranker/ranker.h"
#include "index/score_data.h"

namespace meta {
namespace index {

std::vector<std::pair<doc_id, double>> ranker::score(inverted_index & idx,
                                                 corpus::document & query) const
{
    if(query.counts().empty())
        idx.tokenize(query);

    score_data sd{idx,
                  idx.avg_doc_length(),
                  idx.num_docs(),
                  idx.total_corpus_terms(),
                  query
    };

    using doc_pair = std::pair<doc_id, double>;
    std::vector<doc_pair> results(idx.num_docs());

    for(doc_id i{0}; i < results.size(); ++i) {
        results[i].first = i;
    }

    for(auto & tpair: query.counts())
    {
        term_id t_id{idx.get_term_id(tpair.first)};
        auto pdata = idx.search_primary(t_id);
        sd.doc_count = pdata->counts().size();
        sd.t_id = t_id;
        sd.query_term_count = tpair.second;
        sd.corpus_term_count = idx.total_num_occurences(sd.t_id);
        for(auto & dpair: pdata->counts())
        {
            sd.d_id = dpair.first;
            sd.doc_term_count = dpair.second;
            sd.doc_size = idx.doc_size(dpair.first);
            sd.doc_unique_terms = idx.unique_terms(dpair.first);
            results[dpair.first].second += score_one(sd);
        }
    }

    std::sort(results.begin(), results.end(),
        [](const doc_pair & a, const doc_pair & b) {
            return a.second > b.second;
        }
    );

    return results;
}

}
}
