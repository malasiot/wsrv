$(document).ready(function() {
    const $grid = $('#routes-grid-container');
    const isAdmin = $('#routes-page-wrapper').data('is-admin') === true;

    // Helper: Maps difficulty level text strings directly into corresponding css classes
    function getDifficultyBadgeClass(difficulty) {
        switch (String(difficulty).toLowerCase()) {
            case 'easy': return 'bg-success';
            case 'medium': return 'bg-warning text-dark';
            case 'hard': return 'bg-danger';
            default: return 'bg-secondary';
        }
    }

    // CORE METHOD: Fetches and dynamically renders route card entries
    function fetchRoutes(filters = {}) {
        // Show loading state placeholder
        $grid.html(`
            <div class="col-12 text-center py-5 text-muted" id="routes-loading-spinner">
                <div class="spinner-border text-primary mb-2" role="status"></div>
                <p class="small mb-0">Filtering track records...</p>
            </div>
        `);

        $.ajax({
            url: '/api/routes/list', // Your backend endpoint returning JSON array
            type: 'GET',
            data: filters, // Cleanly appends your future filter options (?difficulty=easy&search=loop)
            dataType: 'json',
            success: function(response) {
                // Expects array: { success: true, data: [ {id:1, title:"", length:5.4, duration:120, difficulty:"Hard"}, ... ] }
                if (response.success && response.data.length > 0) {
                    let cardsHtml = '';

                    $.each(response.data, function(idx, route) {
                        const badgeClass = getDifficultyBadgeClass(route.difficulty);
                        
                        // Dynamic Pace Math calculation safety logic
                        const routeLength = parseFloat(route.length) || 1;
                        const pace = ((parseInt(route.duration) || 0) / (routeLength || 1)).toFixed(1);

                        // Conditional template creation based on client authorization status
                        const editButtonHtml = isAdmin 
                            ? `<a class="btn btn-warning btn-sm edit-route-inline-btn" title="Quick Edit Properties" href="/routes/edit/${route.id}"><i class="bi bi-pencil-square"></i></a>` 
                            : '';

                        cardsHtml += `
                            <div class="col-12 col-md-6 col-lg-4 route-card-wrapper" data-route-id="${route.id}">
                                <div class="card h-100 shadow-sm border-0 position-relative transition-hover">
                                    <div class="position-absolute top-0 end-0 m-3 z-index-2">
                                        <span class="badge ${badgeClass} text-uppercase px-2.5 py-1.5 small tracking-wider shadow-sm route-difficulty-badge">
                                            ${route.difficulty}
                                        </span>
                                    </div>
                                    <div class="card-body d-flex flex-column pt-4.5">
                                        <h4 class="card-title text-primary text-truncate mb-3 pe-5 route-title-text">${route.title}</h4>
                                        <div class="row g-2 text-muted small border-top border-bottom py-3 my-3 bg-light rounded px-1">
                                            <div class="col-4 border-end text-center">
                                                <div class="fw-bold text-dark-emphasis mb-0.5"><i class="bi bi-signpost-split me-1"></i>Dist</div>
                                                <span class="route-length-text">${route.length}</span> km
                                            </div>
                                            <div class="col-4 border-end text-center">
                                                <div class="fw-bold text-dark-emphasis mb-0.5"><i class="bi bi-clock me-1"></i>Time</div>
                                                <span class="route-duration-text">${route.duration}</span> mins
                                            </div>
                                            <div class="col-4 text-center">
                                                <div class="fw-bold text-dark-emphasis mb-0.5"><i class="bi bi-activity me-1"></i>Pace</div>
                                                <span>~${pace}</span> m/k
                                            </div>
                                        </div>
                                        <div class="d-flex justify-content-between align-items-center mt-auto pt-2">
                                            <a href="/route/details/${route.id}" class="btn btn-outline-primary btn-sm flex-grow-1 me-2">
                                                View Details<i class="bi bi-arrow-right-short ms-1"></i>
                                            </a>
                                            ${editButtonHtml}
                                        </div>
                                    </div>
                                </div>
                            </div>`;
                    });

                    $grid.html(cardsHtml);
                } else {
                    // Empty configuration state view matching fallback rules
                    $grid.html(`
                        <div class="col-12 text-center py-5 text-muted border rounded bg-white">
                            <i class="bi bi-map fs-1 text-light-emphasis mb-3 d-block"></i>
                            <h5>No trails found</h5>
                            <p class="small text-muted">Try adjusting your filters or create a new route tracking index file.</p>
                        </div>
                    `);
                }
            },
            error: function() {
                $grid.html(`
                    <div class="col-12 text-center py-5 text-danger border rounded bg-white">
                        <i class="bi bi-exclamation-triangle fs-1 mb-3 d-block"></i>
                        <h5>Unable to connect</h5>
                        <p class="small text-muted">Failed to gather active route directory registries.</p>
                    </div>
                `);
            }
        });
    }

    // 3. INITIALIZATION CALL ON PAGE LOAD
    fetchRoutes();

    // Re-expose the fetch function globally so future filtering modules can trigger updates
    window.RouteEngine = { refresh: fetchRoutes };
});