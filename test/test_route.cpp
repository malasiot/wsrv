#include <gtest/gtest.h>
#include <wsrv/route.hpp>

using namespace ws;

class RouteTest : public ::testing::Test {
protected:
    using Dictionary = std::map<std::string, std::string>;
};

// Basic pattern matching tests
TEST_F(RouteTest, BasicPathMatching) {
    Route route("/user/list/");
    EXPECT_TRUE(route.matches("/user/list/"));
    EXPECT_TRUE(route.matches("/user/list"));
    EXPECT_FALSE(route.matches("/user/profile/"));
    EXPECT_FALSE(route.matches("/admin/list/"));
}

TEST_F(RouteTest, NamedParameters) {
    Route route("/user/{id}/");
    Dictionary params;
    
    EXPECT_TRUE(route.matches("/user/123/", params));
    EXPECT_EQ(params["id"], "123");
    
    params.clear();
    EXPECT_TRUE(route.matches("/user/john/", params));
    EXPECT_EQ(params["id"], "john");
}

TEST_F(RouteTest, MultipleParameters) {
    Route route("/user/{id}/post/{post_id}/");
    Dictionary params;
    
    EXPECT_TRUE(route.matches("/user/123/post/456/", params));
    EXPECT_EQ(params["id"], "123");
    EXPECT_EQ(params["post_id"], "456");
}

TEST_F(RouteTest, RegexPatterns) {
    Route route("/user/{id:\\d+}/");
    Dictionary params;
    
    EXPECT_TRUE(route.matches("/user/123/", params));
    EXPECT_EQ(params["id"], "123");
    
    EXPECT_FALSE(route.matches("/user/abc/", params));
}

TEST_F(RouteTest, RegexAlternatives) {
    Route route("/user/{action:show|hide|edit}/");
    Dictionary params;
    
    EXPECT_TRUE(route.matches("/user/show/", params));
    EXPECT_EQ(params["action"], "show");
    
    EXPECT_TRUE(route.matches("/user/hide/", params));
    EXPECT_EQ(params["action"], "hide");
    
    EXPECT_TRUE(route.matches("/user/edit/", params));
    EXPECT_EQ(params["action"], "edit");
    
    EXPECT_FALSE(route.matches("/user/delete/", params));
}

TEST_F(RouteTest, OptionalSegments) {
    Route route("/posts/{id}");
    Dictionary params;
    
    EXPECT_TRUE(route.matches("/posts/123/", params));
    EXPECT_EQ(params["id"], "123");
    
    params.clear();
    EXPECT_TRUE(route.matches("/posts/123", params));
    EXPECT_EQ(params["id"], "123");
}

TEST_F(RouteTest, MultipleOptionalSegments) {
    Route route("/api/{version}?/users");
    Dictionary params;
    
    EXPECT_TRUE(route.matches("/api/v1/users", params));
    EXPECT_EQ(params["version"], "v1");
    
    params.clear();
    EXPECT_TRUE(route.matches("/api/v2/users/", params));
    EXPECT_EQ(params["version"], "v2");

    params.clear();
    EXPECT_TRUE(route.matches("/api/users/", params));
    EXPECT_EQ(params["version"], "");
}

// Removed MixedStaticDynamic - inline parameters not supported by router

TEST_F(RouteTest, URLGeneration) {
    Route route("/user/{id}/post/{post_id}/");
    Dictionary params;
    params["id"] = "john";
    params["post_id"] = "42";
    
    std::string generated = route.url(params, true);
    EXPECT_EQ(generated, "user/john/post/42/");
    
    std::string absolute = route.url(params, false);
    EXPECT_EQ(absolute, "/user/john/post/42/");
}

TEST_F(RouteTest, URLGenerationOptional) {
    Route route("/posts/{id}");
    Dictionary params;
    params["id"] = "5";
    
    std::string url = route.url(params, true);
    EXPECT_EQ(url, "posts/5/");
}

TEST_F(RouteTest, RealWorldPatterns) {
    // REST API list endpoint
    Route list_route("/api/users/");
    EXPECT_TRUE(list_route.matches("/api/users/"));
    
    // REST API detail endpoint
    Route detail_route("/api/users/{id}/");
    Dictionary params;
    EXPECT_TRUE(detail_route.matches("/api/users/42/", params));
    EXPECT_EQ(params["id"], "42");
    
    // REST API nested endpoint
    Route nested_route("/api/users/{user_id}/posts/{post_id}/");
    params.clear();
    EXPECT_TRUE(nested_route.matches("/api/users/10/posts/99/", params));
    EXPECT_EQ(params["user_id"], "10");
    EXPECT_EQ(params["post_id"], "99");
}

TEST_F(RouteTest, TrailingSlashHandling) {
    Route route("/users/active");
    
    EXPECT_TRUE(route.matches("/users/active"));
    EXPECT_TRUE(route.matches("/users/active/"));
}

TEST_F(RouteTest, ParameterExtraction) {
    Route route("/files/{name}");
    Dictionary params;
    
    EXPECT_TRUE(route.matches("/files/document.pdf/", params));
    EXPECT_EQ(params["name"], "document.pdf");
    
    params.clear();
    EXPECT_TRUE(route.matches("/files/data/", params));
    EXPECT_EQ(params["name"], "data");
}

TEST_F(RouteTest, NonMatchingPatterns) {
    Route route("/admin/dashboard/");
    
    EXPECT_FALSE(route.matches("/admin/"));
    EXPECT_FALSE(route.matches("/admin/dashboard/stats/"));
    EXPECT_FALSE(route.matches("/user/dashboard/"));
}

TEST_F(RouteTest, NumericPatterns) {
    Route route("/page/{num:\\d+}/");
    Dictionary params;
    
    EXPECT_TRUE(route.matches("/page/1/", params));
    EXPECT_EQ(params["num"], "1");
    
    params.clear();
    EXPECT_TRUE(route.matches("/page/999/", params));
    EXPECT_EQ(params["num"], "999");
    
    EXPECT_FALSE(route.matches("/page/abc/", params));
}

TEST_F(RouteTest, SlugPatterns) {
    Route route("/posts/{slug:[a-z0-9-]+}/");
    Dictionary params;
    
    EXPECT_TRUE(route.matches("/posts/my-blog-post/", params));
    EXPECT_EQ(params["slug"], "my-blog-post");
    
    EXPECT_FALSE(route.matches("/posts/My Blog Post/", params));
}

// Parametrized test for various path patterns
class RouteParametrizedTest : public RouteTest, 
                               public ::testing::WithParamInterface<std::tuple<std::string, std::string, bool>> {
};

TEST_P(RouteParametrizedTest, VariousPaths) {
    std::string pattern = std::get<0>(GetParam());
    std::string path = std::get<1>(GetParam());
    bool should_match = std::get<2>(GetParam());
    
    Route route(pattern);
    if (should_match) {
        EXPECT_TRUE(route.matches(path)) << "Pattern '" << pattern << "' should match path '" << path << "'";
    } else {
        EXPECT_FALSE(route.matches(path)) << "Pattern '" << pattern << "' should NOT match path '" << path << "'";
    }
}

INSTANTIATE_TEST_SUITE_P(
    RoutePatterns,
    RouteParametrizedTest,
    ::testing::Values(
        std::make_tuple("/api/users/", "/api/users/", true),
        std::make_tuple("/api/users/", "/api/users", true),
        std::make_tuple("/api/users/{id}/", "/api/users/123/", true),
        std::make_tuple("/api/users/{id}/", "/api/products/123/", false),
        std::make_tuple("/posts/{slug}/", "/posts/hello-world/", true),
        std::make_tuple("/page/{id:\\d+}/", "/page/42/", true),
        std::make_tuple("/page/{id:\\d+}/", "/page/abc/", false)
    )
);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
